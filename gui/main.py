#!/usr/bin/env python

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('Vte', '2.91')
from gi.repository import Gtk, Vte
from gi.repository import GLib
import os
from neovim import attach
import threading
import time
import liblo

win = Gtk.Window()
win.connect('delete-event', Gtk.main_quit)

global num_panes
num_panes = 0

def make_pane():
    global num_panes
    paned = Gtk.Paned.new(Gtk.Orientation.VERTICAL)

    nvim_socket = "/tmp/nvim_worker_" + str(num_panes)
    try:
        os.remove(nvim_socket)
    except:
        pass

    editor = Vte.Terminal()
    editor.spawn_sync(
            Vte.PtyFlags.DEFAULT,
            os.getcwd(),
            ["/usr/bin/nvim"],
            ["NVIM_LISTEN_ADDRESS=" + nvim_socket],
            GLib.SpawnFlags.DO_NOT_REAP_CHILD,
            None,
            None,
            )

    terminal = Vte.Terminal()
    terminal.spawn_sync(
            Vte.PtyFlags.DEFAULT,
            os.getcwd(),
            [os.path.dirname(os.path.realpath(__file__)) + "/../worker/worker", "localhost", "9000", str(num_panes)],
            [],
            GLib.SpawnFlags.DO_NOT_REAP_CHILD,
            None,
            None,
            )

    paned.add1(editor)
    paned.add2(terminal)
    paned.set_wide_handle(True)
    paned.set_position(win.get_size().height * 2 / 3)

    num_panes += 1

    return paned

add_button = Gtk.Button(None, image=Gtk.Image(stock=Gtk.STOCK_ADD))
add_button.show_all()

notebook = Gtk.Notebook()
notebook.set_action_widget(add_button, Gtk.PackType.END)

def setup_nvim(id):
    time.sleep(1)
    nvim_socket = "/tmp/nvim_worker_" + str(id)
    nvim = attach('socket', path=nvim_socket)
    nvim.command('set filetype=python')
    nvim.command('noremap <F4> mmvipy:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('inoremap <F4> mmvipy:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('inoremap <F5> <Esc>yy:call rpcnotify(%d, "exec")<CR>a' % nvim.channel_id)
    nvim.command('noremap <F5> yy:call rpcnotify(%d, "exec")<CR>' % nvim.channel_id)
    nvim.command('xnoremap <F5> mmy:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('inoremap <F6> mmggyG:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('noremap <F6> mmggyG:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    addr = liblo.Address("localhost", 9000 + id + 1, liblo.UDP)
    while True:
        event = nvim.next_message()
        code = nvim.funcs.getreg('*')
        liblo.send(addr, "exec", code)


def add_pane(a, b, c=None):
    id = notebook.get_n_pages()
    id = num_panes
    pane = make_pane()
    notebook.append_page(pane, Gtk.Label(str(id)))
    notebook.show_all()
    notebook.set_current_page(notebook.page_num(pane))
    notebook.show_all()
    threading.Thread(target=setup_nvim, args=(id,)).start()

add_button.connect("clicked", add_pane, None)

win.add(notebook)
win.show_all()

add_pane(None, None)
def resize(a, b, c):
    for child in notebook.get_children():
        child.set_position(win.get_size().height * 2 / 3)
win.connect("visibility-notify-event", resize, None)

Gtk.main()
