#include "JuliaProcessor.h"
#include <iostream>
#include <julia.h>

class JuliaException : public std::exception
{
  public:
    JuliaException(const char* msg)
      : msg(msg)
    {
    }
    virtual const char* what() const throw() { return msg; }
  private:
    const char* msg;
};

void
check_err()
{
    if (jl_exception_occurred()) {
        // call julia's built-in string method on the exception and rethrow it
        jl_function_t* func = jl_get_function(jl_base_module, "string");
        jl_value_t* argument = jl_exception_occurred();
        jl_value_t* ret = jl_call1(func, argument);
        throw JuliaException(jl_string_ptr(ret));
    }
}

JuliaProcessor::JuliaProcessor()
{
    // spawn processing thread
    std::thread([&]() {
        Frame frame;

        while (true) {
            // wait for a frame of audio
            inBuffer.wait_dequeue(frame);
            langThreadExec([&]() {
                try {
                    // get handle to user process fn
                    jl_function_t* func =
                      jl_get_function(jl_main_module, "process");
                    check_err();
                    if (!func)
                        return;

                    jl_array_t *inArray = nullptr, *outArray = nullptr;
                    JL_GC_PUSH2(&inArray, &outArray);

                    // alloc in/out
                    jl_value_t* array_type =
                      jl_apply_array_type(jl_float32_type, 2);
                    check_err();
                    inArray = jl_alloc_array_2d(array_type, frame.numInChannels,
                                                frame.size);
                    check_err();
                    outArray = jl_alloc_array_2d(
                      array_type, frame.numOutChannels, frame.size);
                    check_err();

                    // fill in data
                    float* inData = (float*)jl_array_data(inArray);
                    check_err();
                    std::copy(frame.data.begin(), frame.data.end(), inData);

                    // call process fn
                    jl_call2(func, (jl_value_t*)inArray, (jl_value_t*)outArray);
                    check_err();

                    // copy out data
                    float* outData = (float*)jl_array_data(outArray);
                    check_err();
                    if (!outBuffer.try_enqueue(
                          Frame(std::vector<float>(
                                  outData,
                                  outData + frame.size * frame.numOutChannels),
                                frame.numInChannels, frame.numOutChannels,
                                frame.size))) {
                        std::cerr
                          << "WARNING: No space for frame on output queue!"
                          << std::endl;
                    }

                    JL_GC_POP();
                } catch (JuliaException e) {
                    std::cerr << "Julia process error:\n"
                              << e.what() << std::endl;
                    silence();
                }
            }).wait();
        }
    }).detach();

    // enqueue julia init call
    langThreadExec([]() {
        try {
            jl_init(JULIA_INIT_DIR);
            check_err();
        } catch (JuliaException e) {
            std::cerr << "Julia init error:\n" << e.what() << std::endl;
        }
    }).wait();

    // silence by default
    silence();
}

JuliaProcessor::~JuliaProcessor()
{
    // julia cleanup
    langThreadExec([]() {
        try {
            jl_atexit_hook(0);
            check_err();
        } catch (JuliaException e) {
            std::cerr << "Julia cleanup error:\n" << e.what() << std::endl;
        }
    }).wait();
}

void
JuliaProcessor::process(size_t numInChannels, size_t numOutChannels,
                        size_t frameSize, float** inBufs, float** outBufs)
{
    // clear possibly uninitialized output
    for (size_t i = 0; i < numOutChannels; i++)
        std::fill(outBufs[i], outBufs[i] + frameSize, 0);

    // enqueue input data for processing
    std::vector<float> dry(frameSize * numInChannels);
    for (size_t j = 0; j < frameSize; j++)
        for (size_t i = 0; i < numInChannels; i++)
            dry[i + j * numInChannels] = inBufs[i][j];
    if (!inBuffer.try_enqueue(
          Frame(dry, numInChannels, numOutChannels, frameSize)))
        std::cerr << "WARNING: No space for frame on input queue!" << std::endl;

    // pop processed output data, if any
    Frame wet;
    if (outBuffer.try_dequeue(wet) && wet.size == frameSize &&
        wet.numInChannels == numInChannels &&
        wet.numOutChannels == numOutChannels) {
        for (size_t j = 0; j < frameSize; j++)
            for (size_t i = 0; i < numOutChannels; i++)
                outBufs[i][j] = wet.data[i + j * numOutChannels];
    } else
        std::cerr << "Buffer underrun!" << std::endl;
}

void
JuliaProcessor::init()
{
}

void
JuliaProcessor::exec(std::string code)
{
    // exec user code
    langThreadExec([=]() {
        try {
            jl_eval_string(code.c_str());
            check_err();
        } catch (JuliaException e) {
            std::cerr << "Julia exec error:\n" << e.what() << std::endl;
        }
    });
}

void
JuliaProcessor::silence()
{
    // silence just redefines the process function to output silence
    // as far as hacky solutions go, this one isn't that bad once you consider
    // the other options for achieving the same thing
    langThreadExec([]() {
        try {
            jl_eval_string(
              "function process(in::Array{Float32}, out::Array{Float32})\n"
              "    out[:] = 0\n"
              "end");
            check_err();
        } catch (JuliaException e) {
            std::cerr << "Julia silence error:\n" << e.what() << std::endl;
        }
    });
}
