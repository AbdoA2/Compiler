import tkinter
import tkinter.ttk as ttk
from tkinter import Entry, Label, Tk, Frame, Button
from tkinter.filedialog import askopenfilename, asksaveasfile, askdirectory
from tkinter.messagebox import askyesno
from tkinter.simpledialog import askstring
import subprocess
from os import listdir, getcwd, remove
from os.path import isfile, join, isdir, splitext
from ntpath import basename
from shutil import rmtree


class FindReplaceDialog(Frame):
    def __init__(self, editor):
        self.root = Tk()
        Frame.__init__(self, self.root)
        self.pack(side="top", fill="both", expand=True)
        self.editor = editor
        self.createBody()
        self.root.resizable(width=False, height=False)
        self.root.title("Find & Replace")
        self.root.mainloop()

    def createBody(self):
        Label(self, text="Find:").grid(row=0, column=0)
        Label(self, text="Replace:").grid(row=1, column=0)

        self.find_entry = Entry(self)
        self.find_entry.grid(row=0, column=1, sticky='ew')
        self.replace_entry = Entry(self)
        self.replace_entry.grid(row=1, column=1, sticky='ew')
        
        Button(self, text="Replace All", command=self.replace).grid(row=2, column=0)
        Button(self, text="Close").grid(row=2, column=1)

        self.columnconfigure(0, weight=1)
        self.columnconfigure(1, weight=5)

    def replace(self):
        text = self.editor.tabs[self.editor.current_file][1]
        text2edit = text.get(1.0, tkinter.END)
        t = text2edit.replace(self.find_entry.get(), self.replace_entry.get())
        text.delete(1.0, tkinter.END)
        text.insert(tkinter.INSERT, t)
        if self.editor.current_file.endswith('.c'):
            self.editor.highlight(text, self.editor.keywords)




class StatusBar(tkinter.Frame):   
    def __init__(self, master):
        tkinter.Frame.__init__(self, master)
        self.position=tkinter.StringVar()        
        self.label_cursor=tkinter.Label(self, bd=1, anchor=tkinter.W,
                           textvariable=self.position,
                           font=('arial',12,'normal'))
        self.position.set('Status Bar')
        self.label_cursor.pack()


class MainWindow(tkinter.Frame):

    def __init__(self, parent):
        self.parent = parent
        self.tag_ctr = 0
        self.new_count = 1
        self.programText = ""
        self.tabs = dict()
        self.current_file = ""
        self.current_folder = getcwd()
        k = ['if', 'else', 'while', 'do', 'function', 'void', 'int', 'switch', 'for',
            'return', 'default', 'case', 'char', 'float', 'const', 'repeat', 'until', 'break', 'continue'];
        self.keywords = dict(zip(k, ['red'] * len(k)))
        self.errors = {'error': 'red', 'semantic': 'blue', 'syntax': 'green', 'warning': 'orange'}
        k = ['add', 'add.f', 'mul', 'beq', 'bne', 'slt', 'sgt', 'ld', 'li.f', 'li', 'mov', 'div', 'j', 'call'];
        self.asm_keywords = dict(zip(k, ['blue'] * len(k)))
        tkinter.Frame.__init__(self, parent)
        self.createMenus()
        self.createBody()
        self.open_project(self.current_folder)

    def createMenus(self):
        # create the menubar
        menubar = tkinter.Menu(self.parent)

        #create file menu
        filemenu = tkinter.Menu(menubar, tearoff=0)
        filemenu.add_command(label="Open Project", command=self.open_project)
        filemenu.add_separator()
        filemenu.add_command(label="New Program", command=self.new_program)
        filemenu.add_command(label="Load Program", command=self.load_program)
        filemenu.add_command(label="Remove File", command=self.remove_file)
        filemenu.add_separator()
        filemenu.add_command(label="Save", command=self.save)
        filemenu.add_command(label="Save As", command=self.save_as)
        filemenu.add_command(label="Close Tab", command=self.close)
        filemenu.add_separator()
        filemenu.add_command(label="Close Tab", command=self.close)
        filemenu.add_command(label="Exit", command=exit)
        menubar.add_cascade(label="File", menu=filemenu)

        # create edit menu
        editmenu = tkinter.Menu(menubar, tearoff=0)
        editmenu.add_command(label="Find Replace", command=self.find_replace)
        menubar.add_cascade(label="Edit", menu=editmenu)

        # create code menu
        codemenu = tkinter.Menu(menubar, tearoff=0)
        codemenu.add_command(label="Compile", command=self.check_syntax)
        menubar.add_cascade(label="Code", menu=codemenu)

        # add the menus to the root window
        self.parent.config(menu=menubar)

    def createBody(self):
        # create the main notebook
        self.notebook = tkinter.ttk.Notebook(self)
        self.notebook.grid(row=0, column=2, columnspan=2, sticky="nsew")
        self.notebook.bind('<<NotebookTabChanged>>', self.tab_changed)

        # create folder tree
        self.folder_tree = tkinter.ttk.Treeview(self)
        self.folder_tree.grid(row=0, column=0, rowspan=2, sticky='nsew')
        self.folder_tree.bind('<Double-1>', self.file_selected)

        self.scroll1 = tkinter.Scrollbar(self, command=self.folder_tree.yview)
        self.scroll1.grid(row=0, column=1, rowspan=2, sticky='ns')
        self.folder_tree['yscrollcommand'] = self.scroll1.set

        # create ouput window
        self.out = tkinter.Text(self, height=10)
        self.out.grid(row=1, column=2, sticky="nsew")
        self.out.config(state=tkinter.DISABLED)

        # create scrollbar
        self.scroll2 = tkinter.Scrollbar(self, command=self.out.yview)
        self.scroll2.grid(row=1, column=3, sticky='ns')
        self.out['yscrollcommand'] = self.scroll2.set

        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)
        self.columnconfigure(2, weight=5)

        # status bar
        self.status_bar = StatusBar(self.parent)
        self.status_bar.pack(side=tkinter.BOTTOM)

        
    def find_replace(self):
        if self.current_file != '':
            FindReplaceDialog(self)

    def close_tab(self, filename):
        if filename == '':
            return
        i = self.tabs[filename][0]
        self.notebook.forget(i)
        self.tabs.pop(filename)
        if len(self.tabs) > 0:
            for k in self.tabs:
                if self.tabs[k][0] > i:
                    self.tabs[k][0] -= 1

    def close(self):
        close_tab(self.current_file)


    def update_cursor(self, e):
        i = self.tabs[self.current_file][1].index(tkinter.INSERT)
        line, col = map(int, i.split('.'))
        self.tabs[self.current_file][2] = line
        self.tabs[self.current_file][3] = col
        self.status_bar.position.set("Line: %d, Col: %d" % (line, col))

    def key_event(self, p):
        self.update_cursor(p.widget)
        if self.current_file.endswith('.c'):
            self.highlight(p.widget, self.keywords)

    def highlight(self, t, keywords):
        # remove tags
        for tag in t.tag_names():
            t.tag_delete(tag)

        lines = str(t.get("1.0", tkinter.END)).split('\n')
        for i in range(len(lines)):
            col = 0
            while col < len(lines[i]):
                start = end = col
                while end < len(lines[i]) and (lines[i][end].isalnum() or lines[i][end] == '_'):
                    end += 1

                word = lines[i][start:end]
                if word in keywords:
                    t.tag_add(str(self.tag_ctr), "%d.%d" % (i+1, start), "%d.%d" % (i+1, end))
                    t.tag_config(str(self.tag_ctr), foreground=keywords[word])
                    self.tag_ctr += 1
                
                while end < len(lines[i]) and not lines[i][end].isalnum():
                    end += 1
                col = end


    def tab(self, arg):
        self.tabs[self.current_file][1].insert(tkinter.INSERT, " " * 4)
        return 'break'

    def load_program(self):
        filename = askopenfilename()
        if filename:
            self.new_tab(filename)

    def save_as(self):
        if self.current_file == '':
            return

        f = asksaveasfile(mode='w', defaultextension=".c", filetypes=[("3AM C Files", "*.c")])
        if f is None: # asksaveasfile return `None` if dialog closed with "cancel".
            return
        text2save = str(self.tabs[self.current_file][1].get(1.0, tkinter.END)) # starts from `1.0`, not `0.0`
        f.write(text2save)
        f.close()

    def new_program(self):
        filename = askstring("New Program Name", "Enter the new program name.")
        if filename is not None:
            self.current_file = join(self.current_folder, filename + '.c').replace('\\', '/')
            open(self.current_file, 'w+').close()
            self.new_tab(self.current_file)
            if self.current_folder != '':
                self.open_project(dirname=self.current_folder)

    def save(self, e):
        text2save = str(self.tabs[self.current_file][1].get(1.0, tkinter.END)) # starts from `1.0`, not `0.0`
        f = open(self.current_file, 'w+')
        f.write(text2save)
        f.close()

    def remove_file(self):
        filename = self.get_selected_filename()
        ok = askyesno("Removing File", "Are you sure you want to delete\n %s" % filename)

        if not ok:
            return

        if filename in self.tabs:
            self.close_tab(filename)

        if isfile(filename):
            remove(filename)
        else:
            rmtree(filename)
        self.open_project(self.current_folder)

    def display_out(self, err):
        self.out.config(state=tkinter.NORMAL)
        self.out.delete(1.0, tkinter.END)
        self.out.insert(tkinter.INSERT, err)
        self.out.config(state=tkinter.DISABLED)
        self.highlight(self.out, self.errors)

    def check_syntax(self):
        text = str(self.tabs[self.current_file][1].get(1.0, tkinter.END))
        p = subprocess.Popen("syntax.exe", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        res, err = p.communicate(str.encode(text))
        self.display_out(err)
        if err.decode('ascii').find('error') < 0:
            self.compile(text)

    def compile(self, text):
        p = subprocess.Popen("compiler.exe", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        res, err = p.communicate(str.encode(text))
        self.display_out(err)
        if err.decode('ascii').find('error') < 0:
            f = open(splitext(basename(self.current_file))[0] + '.asm', 'w+');
            f.write(res.decode('ascii').replace('\r\n', '\n'))
            f.close()
            if self.current_folder != '':
                self.open_project(dirname=self.current_folder)


    def open_folder(self, path, parent):
        for f in listdir(path):
            abspath = join(path, f)
            if isdir(abspath):
                parent_element = self.folder_tree.insert(parent, 'end', text=f, open=False)
                self.open_folder(abspath, parent_element)

        for f in listdir(path):
            abspath = join(path, f)
            if (not isdir(abspath) and (abspath.endswith('.c') or abspath.endswith('.asm'))):
                parent_element = self.folder_tree.insert(parent, 'end', text=f)

    def open_project(self, dirname=''):
        dirname = askdirectory() if dirname == '' else dirname
        self.current_folder = dirname
        for i in self.folder_tree.get_children():
            self.folder_tree.delete(i)
        root = self.folder_tree.insert('', 'end', text=dirname, open=True)
        self.open_folder(dirname, root)

    def get_selected_filename(self):
        item = self.folder_tree.selection()[0]
        filename = self.folder_tree.item(item,"text")
        parent = self.folder_tree.parent(item)
        while parent:
            filename = join(self.folder_tree.item(parent,"text"), filename)
            parent = self.folder_tree.parent(parent)
        return filename.replace('\\', '/')

    def file_selected(self, e):
        filename = self.get_selected_filename()

        if not filename in self.tabs:
           self.new_tab(filename)

        self.notebook.select(tab_id=self.tabs[filename][0])
        self.current_file = filename
        self.update_cursor("")

    def new_tab(self, filename):
        f = tkinter.ttk.Frame(self.notebook)
        name = basename(filename)
        self.notebook.add(f, text=name)
        t = tkinter.Text(f)
        t.pack(side=tkinter.LEFT, fill=tkinter.BOTH, expand=1)
        t.bind('<KeyRelease>', self.key_event)
        t.bind('<ButtonRelease-1>', self.update_cursor)
        t.bind('<Control-s>', self.save)

        scrollb = tkinter.Scrollbar(f, command=t.yview)
        scrollb.pack(side=tkinter.RIGHT, fill=tkinter.Y)
        t['yscrollcommand'] = scrollb.set

        if len(filename):
            t.insert(tkinter.INSERT, open(filename).read())
            if filename.endswith('.c'):
                self.highlight(t, self.keywords)
            elif filename.endswith('.asm'):
                self.highlight(t, self.asm_keywords)
            self.tabs[filename] = [len(self.tabs), t, 0, 0]
            self.current_file = filename


    def tab_changed(self, e):
        if len(self.tabs) > 0:
            i = self.notebook.index(self.notebook.select())
            for tab in self.tabs:
                if self.tabs[tab][0] == i:
                    self.current_file = tab
                    break
            self.update_cursor("")



root = tkinter.Tk()
window = MainWindow(root)
window.pack(fill=tkinter.BOTH, expand=2)
root.title("3AM Compiler")
root.mainloop()