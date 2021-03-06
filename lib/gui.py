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
import grpc
import worker_pb2_grpc
import worker_pb2

win = Gtk.Window()
win.connect('delete-event', Gtk.main_quit)

num_panes = 0

def make_pane(language):
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
            #["/usr/bin/gdb", "--args", os.path.dirname(os.path.realpath(__file__)) + "/../worker/worker", "127.0.0.1", str(9000 + num_panes), language],
            [os.path.dirname(os.path.realpath(__file__)) + "/../worker/worker", "127.0.0.1", str(9000 + num_panes), language],
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

def setup_nvim(id,language):
    time.sleep(4)
    nvim_socket = "/tmp/nvim_worker_" + str(id)
    nvim = attach('socket', path=nvim_socket)
    nvim.command('set filetype=' + language)
    nvim.command('noremap <F4> mmvipy:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('inoremap <F4> <Esc>mmvipy:call rpcnotify(%d, "exec")<CR>`ma' % nvim.channel_id)
    nvim.command('inoremap <F5> <Esc>yy:call rpcnotify(%d, "exec")<CR>a' % nvim.channel_id)
    nvim.command('noremap <F5> yy:call rpcnotify(%d, "exec")<CR>' % nvim.channel_id)
    nvim.command('xnoremap <F5> mmy:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('inoremap <F6> <Esc>mmggyG:call rpcnotify(%d, "exec")<CR>`ma' % nvim.channel_id)
    nvim.command('noremap <F6> mmggyG:call rpcnotify(%d, "exec")<CR>`m' % nvim.channel_id)
    nvim.command('noremap <F12> :call rpcnotify(%d, "silence")<CR>' % nvim.channel_id)
    nvim.command('inoremap <F12> <Esc>:call rpcnotify(%d, "silence")<CR>a' % nvim.channel_id)
    nvim.command('noremap <F11> :call rpcnotify(%d, "clearBuffer")<CR>' % nvim.channel_id)
    nvim.command('inoremap <F11> <Esc>:call rpcnotify(%d, "clearBuffer")<CR>a' % nvim.channel_id)
    channel = grpc.insecure_channel('localhost:' + str(9000 + id))
    stub = worker_pb2_grpc.WorkerStub(channel)
    stub.SetupAPI(worker_pb2.Empty())
    try:
        while True:
            event = nvim.next_message()
            if len(event) == 3:
                if event[1] == "exec":
                    code = nvim.funcs.getreg('*')
                    stub.Exec(worker_pb2.Code(code=code))
                elif event[1] == "silence":
                    stub.Silence(worker_pb2.Empty())
                elif event[1] == "clearBuffer":
                    stub.ClearBuffer(worker_pb2.Empty())
    except:
        pass

def add_pane():
    language = "python"
    id = notebook.get_n_pages()
    id = num_panes
    pane = make_pane(language)
    label = Gtk.HBox()
    label.pack_start(Gtk.Label(language + "-" + str(id)), True, True, 0)
    close_button = Gtk.Button(None, image=Gtk.Image(stock=Gtk.STOCK_CLOSE))
    close_button.set_relief(Gtk.ReliefStyle.NONE)
    def close_tab(a, b):
        notebook.remove_page(notebook.page_num(pane))
    close_button.connect("clicked", close_tab, None)
    label.pack_start(close_button, True, True, 0)
    label.show_all()
    notebook.append_page(pane, label)
    notebook.show_all()
    notebook.set_current_page(notebook.page_num(pane))
    notebook.show_all()
    threading.Thread(target=setup_nvim, args=(id,language), daemon=True).start()

def add_button_pressed(widget, event):
    add_pane()
add_button.connect("clicked", add_button_pressed, None)

win.add(notebook)
win.show_all()

def resize(a, b, c):
    for child in notebook.get_children():
        child.set_position(win.get_size().height * 2 / 3)
win.connect("visibility-notify-event", resize, None)

# hack because it won't show the header bar if there is no page
notebook.append_page(Gtk.Label(""), Gtk.Label(""))
notebook.show_all()
notebook.remove_page(0)

# http://stackoverflow.com/questions/16410852/keyboard-interrupt-with-with-python-gtk
import signal
signal.signal(signal.SIGINT, signal.SIG_DFL)

Gtk.main()
