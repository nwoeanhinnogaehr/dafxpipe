# what is this?

Software for live coding audio effects and synths in Python. (again)

Take a look at the [examples](examples/).

There's a GUI! After building (see below), run it with `./gui`.

For each tab you open, a JACK device will be created. The bottom window is the console for viewing error messages. The top window is your workspace. Some keys are mapped for you:

* F4 will execute the current "paragraph" (delimited by blank lines).
* F5 will execute the current line or the current visual selection.
* F6 will execute the whole file.
* F12 will silence output.

If you define a function like so:
```python
def process(input, output):
  output[:] = input
```
and attach something to the JACK ports, you should hear the audio flowing through. `input` and `output` are NumPy arrays with shape `(num_channels, buffer_size)`.

# dependencies

* [Python 3](https://www.python.org/)
* [NumPy](http://www.numpy.org/)
* [Boost.Python ≥ 1.6.3](https://github.com/boostorg/python)
* [JACK](http://www.jackaudio.org/)
* [my fork of jackcpp (merged soon hopefully)](https://github.com/nwoeanhinnogaehr/jackcpp)
* [neovim](https://neovim.io/)
* [GTK+ 3](https://www.gtk.org/)
* [PyGObject](https://wiki.gnome.org/Projects/PyGObject)
* [grpc](http://www.grpc.io/)
* [tup](http://gittup.org/tup/)

# build

```
$ tup
```

# license

GPL
