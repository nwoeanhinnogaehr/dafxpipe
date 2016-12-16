# what is this?

Software for live coding audio effects in Python. (again)

There's a GUI! After building (see below), run it with `python3 gui/main.py`.

For each tab you open, a JACK device will be created. The bottom window is the console for viewing error messages. The top window is your workspace. Some keys are mapped for you:

* F4 will execute the current "paragraph" (delimited by blank lines).
* F5 will execute the current line or the current visual selection.
* F6 will execute the whole file.

If you define a function like so:
```python
def process(input, output):
  output[:] = input
```
and attach something to the JACK ports, you should hear the audio flowing through. `input` and `output` are NumPy arrays with shape `(num_channels, buffer_size)`.

That's about it for now. You should be able to do some cool stuff if you import something like [librosa](http://librosa.github.io/librosa/effects.html).

# dependencies

* [Python 3](https://www.python.org/)
* [NumPy](http://www.numpy.org/)
* [Boost.Python ≥ 1.6.3](https://github.com/boostorg/python)
* [JACK](http://www.jackaudio.org/)
* [jackcpp](http://www.x37v.info/projects/jackcpp/)
* [liblo ≥ 3713b9](http://liblo.sourceforge.net/)
* [pyliblo](http://das.nasophon.de/pyliblo/)
* [neovim](https://neovim.io/)
* [GTK+ 3](https://www.gtk.org/)
* [PyGObject](https://wiki.gnome.org/Projects/PyGObject)
* [tup](http://gittup.org/tup/)

# build

```
$ tup
```

# license

GPL
