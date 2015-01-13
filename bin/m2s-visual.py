#!/usr/bin/env python

import sys
import re
import sqlite3
import matplotlib
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as pyplot
import matplotlib.ticker as ticker
from optparse import OptionParser
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2TkAgg
import Tkinter as Tk

# Function to parse the memory trace generated by m2s.
def parse_trace(input):  
    cycle=0
    f=open(input, 'r')

    # Parse each line.
    for line in f:
        if re.search('clk', line ):
            cycle=int(re.search('(\d+)', line).group(1))
        
        # If there is a new memory access, add it to the SQL database.
        elif re.search('mem.new_access ', line ): 
            data=re.search('.*name="A-([\d]+).*type=.(\w+).*addr=(\w+)', line)

            # Pull the data parsed from the file into variables to add to the table.
            access_id=int(data.group(1))
            access_type=data.group(2)
            addr=int(data.group(3), 0)
            
            # SQL command to add a new access.
            c.execute('INSERT OR IGNORE INTO Accesses VALUES(%d, \'%s\', %d, %d, 0);'
                      % (access_id, access_type, addr, cycle))

        # If a memory access is ending, update the line in the SQL database.
        elif re.search('mem.end_access ', line ): 
            access_id=int(re.search('.*name="A-([\d]+)', line).group(1))

            # SQL command to update an access.
            c.execute('UPDATE OR IGNORE Accesses SET CycleEnd = %d WHERE ID=%d;' 
                      % (cycle, access_id))
    f.close

    # This needs to be run to actually update the table.
    conn.commit()

# Function to generate a histogram for a given cycle.
def create_hist(cycle, choice):
    lower_bnd = 0

    # Pull information from the SQL table.
    max_addr = c.execute('SELECT MAX(Address) FROM ACCESSES;').fetchone()
    min_addr = c.execute('SELECT MIN(Address) FROM ACCESSES;').fetchone()    
    max_cycle = c.execute('SELECT MAX(CycleEnd) FROM ACCESSES;').fetchone()

    # Can't display memory information if cycle requested is larger than the max.
    if cycle > max_cycle:
        return

    # Show a range of 1000 cycles.
    if cycle > 10000:
        lower_bnd = cycle - 10000
            
    # Show all accesses, only loads, or only stores, based on user selection.
    if choice is 1: #Show all accesses
        c.execute('SELECT Address FROM ACCESSES WHERE '\
                      '(CycleEnd > %d AND CycleStart < %d);' 
                  % (lower_bnd, cycle))
    elif choice is 2: #Show only load accesses        
        c.execute('SELECT Address FROM ACCESSES WHERE '\
                      '(CycleEnd > %d AND CycleStart < %d) AND Type=\'load\';'
                  % (lower_bnd, cycle)) 
    elif choice is 3: #Show only store accesses
        c.execute('SELECT Address FROM ACCESSES WHERE '\
                      '(CycleEnd > %d AND CycleStart < %d) AND Type=\'store\';'
                  % (lower_bnd, cycle))
    
    # Pull all selected addresses into an array. 
    addr_list = c.fetchall()
    
    # Plot the histogram.
    f = pyplot.figure(1)
    pyplot.clf()
    n, bins, patches = pyplot.hist(addr_list, bins=20)

    # Calculate and display bin size. A bin is the range that each bar of the
    # histogram covers.
    binsize =  bins[1] - bins[0] 
    binSizeTxt.set('Bin Size is %.2f KB' % (binsize/1024))

    # Set axis labels.
    pyplot.xlabel('Addresses')
    pyplot.ylabel('Number of Accesses')
    pyplot.title('Mem Access Frequency (Cycle %d - %d)' % (lower_bnd, cycle))

    # Set default to show entire range of memory.
    pyplot.xlim([min_addr, max_addr])

    # Display x-axis values in hex.
    axes = pyplot.gca()
    axes.get_xaxis().set_major_formatter(ticker.FormatStrFormatter("0x%x"))

    pyplot.grid(True)
    
    # Create tk drawing area so the histogram can be displayed in the GUI.
    global canvas
    if canvas.get_tk_widget() != None:
        canvas.get_tk_widget().pack_forget()

    # Put the histogram in the GUI.
    canvas = FigureCanvasTkAgg(f, master=root)

    # Add the toolbar from the histogram to the GUI.
    global toolbar
    toolbar.pack_forget()
    toolbar = NavigationToolbar2TkAgg(canvas, root)
    toolbar.update()

    # Display the GUI.
    canvas.get_tk_widget().pack(side=Tk.TOP, fill=Tk.BOTH, expand=1)
            
# Function to exit when the quit button is pressed.
def _quit():
    root.quit()     # stops mainloop
    root.destroy()  # this is necessary on Windows to prevent
    # Fatal Python Error: PyEval_RestoreThread: NULL tstate

# Function that will create a new histogram by calling create_hist with the
# correct cycle number and access_type.
def new_histogram():
    access_type = access_type_var.get()
    global current_cycle
    current_cycle = int(cycle_widget.get())
    create_hist(current_cycle, access_type)

# Function to increase the cycle number by 1000 and generate a new histogram.
# Also updates the cycle number in the box.
def forward_histogram():
    access_type = access_type_var.get()
    global current_cycle
    current_cycle = current_cycle + 1000

    global cycle_widget
    cycle_widget.delete(0, Tk.END)
    cycle_widget.insert(0, current_cycle)
    
    create_hist(current_cycle, access_type)

# Function to generate a new histogram if the access type is changed.
def update_type_histogram():
    access_type = access_type_var.get()
    global current_cycle
    create_hist(current_cycle, access_type)
    

"""
        MAIN FUNCTION
"""
"""
Option parsing
"""
# Add command line options to allow the passing of a trace file or a database.
parser = OptionParser()
parser.add_option("-p", "--parse", dest="tracefile",
                  help="Multi2Sim trace file to parse", metavar="TRACEFILE")
parser.add_option("-d", "--database", dest="database_file",
                  help="Database file where our data is")

# Create a database if one was not passed.
(options, args) = parser.parse_args()
if options.tracefile:
    trace = options.tracefile
    conn = sqlite3.connect(trace + '.db')
elif options.database_file:
    conn = sqlite3.connect(options.database_file)
else:
    parser.error("Missing arguments. Please run with '-h' option for help")

"""
Tkinter config
"""
current_cycle = 0

# General GUI settings.
root = Tk.Tk()
root.wm_title("Memory Access Visualization Tool")
root.attributes('-fullscreen', True)
f = pyplot.figure(1)

# Create the canvas to add the histogram to.
canvas = FigureCanvasTkAgg(f, master=root)
# Add the toolbar.
toolbar = NavigationToolbar2TkAgg(canvas, root)

# Add a quit button.
quit_button = Tk.Button(master=root, text='Quit', command=_quit)
quit_button.pack(side=Tk.BOTTOM)

# Add the button to jump 1000 cycles
forward_button = Tk.Button(master=root, text='>> 1000 cycles', command=forward_histogram)
forward_button.pack(side=Tk.BOTTOM)

# The frame is used to put the cycle button, new histogram button, and entry box in
# the right places.
frame = Tk.Frame(root)
typeFrame = Tk.Frame(root)

cycle_label = Tk.Label(master=frame, text='Cyle:')
cycle_label.pack(anchor=Tk.NW)
cycle_button = Tk.Button(master=frame, text='New Histogram', command=new_histogram)
cycle_button.pack(side=Tk.RIGHT)
cycle_widget = Tk.Entry(master=frame)
cycle_widget.pack(side=Tk.LEFT)

# Create the access type header.
access_type_var = Tk.IntVar()
access_type_var.set(1)
access_type_label = Tk.Label(master=typeFrame, text='Choose Access Type: ')
access_type_label.pack(side=Tk.TOP, anchor=Tk.NW)

# Create the access type check boxes.
Tk.Radiobutton(master=typeFrame, text="All", variable=access_type_var, value=1, command=update_type_histogram).pack(side=Tk.TOP, anchor=Tk.NW)
Tk.Radiobutton(master=typeFrame, text="Loads only", variable=access_type_var, value=2, command=update_type_histogram).pack(side=Tk.BOTTOM, anchor=Tk.NW)
Tk.Radiobutton(master=typeFrame, text="Stores only", variable=access_type_var, value=3, command=update_type_histogram).pack(side=Tk.BOTTOM, anchor=Tk.NW)

frame.pack(side=Tk.BOTTOM)
typeFrame.pack(side=Tk.RIGHT)

# Create the Bin Size display.
binSizeTxt = Tk.StringVar()
binSizeTxt.set('Bin Size')
binSize_label = Tk.Label(master=root, fg='blue', bg='yellow', font='Helvetica 16 bold', textvariable=binSizeTxt)
binSize_label.pack(anchor=Tk.SE, side=Tk.BOTTOM)

"""
Database setup
"""
conn.row_factory = lambda cursor, row: row[0]
c = conn.cursor()
c.execute('CREATE TABLE IF NOT EXISTS '\
              'Accesses(ID primary key, Type text, Address Integer,'\
              ' CycleStart Integer, CycleEnd Integer);')
if options.tracefile:
    parse_trace(trace)
 
Tk.mainloop()

conn.close()
