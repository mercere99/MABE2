==========
Quickstart
==========

THIS PAGE IS STILL UNDER CONSTRUCTION!

TODO: Add example to go along with text w/ pics.

The ``.mabe`` File
--------

The ``.mabe`` file is generated from your chosen set of modules, and is the configuration page
that you can use to run your experiments! In the ``.mabe`` file, you can change variables that
have been set up in the different modules connected to the ``.mabe`` file. 

To generate your ``.mabe`` file, first you will want to make sure that you have run the ``make`` command 
since your last updates. To do so, navigate to the build directory (``cd MABE2/build``). From there, run the
command ``make clean ; make``, which will set you up with a clean run. If there are any errors that pop up, 
now is the time to fix them!

Next, you will navigate to the ``settings`` directory inside of ``build``. To do so, first make sure you 
are in the ``build`` directory. From there, run the following command: ``cd settings``. 

From here, you'll want to make sure you can see your .gen file! You can list all of the files inside this 
directory by running ``ls``. You should see a file labled ``[filename].gen``. If not, you can create it by 
running the following command: 

TODO: how to make the .gen file?

Now we're ready to create our ``.mabe`` file! Navigate back up one directory to ``build``, which can be done by 
running: ``cd ..``. From here you will run the command ``./MABE -f settings/[filename].gen -g [filename].mabe``, 
which will generate a ``.mabe`` file named ``[filename].mabe`` with the specifications from the ``[filename].gen`` file. 

Congratulations! You've created your first ``.mabe`` file!

Summary
*****
Step 1: ``make clean ; make`` inside of the ``build`` directory. 

Step 2: ``cd settings`` and then ``ls`` to make sure your ``.gen`` file is there. If not, create it 
using the command: TODO

Step 3: ``cd ..`` and then run ``./MABE -f settings/[filename].gen -g settings/[filename].mabe`` to create the 
``.mabe file``. 


Running the ``.mabe`` File
--------

To run your ``.mabe`` file, navigate to the ``build`` directory. From the ``MABE2`` folder, run 
the following command: ``cd build``. From here, simply run the command ``./MABE -f [filename].mabe``. 


Changing the ``.mabe`` File
--------
You can modify your experiment by changing the variable values inside of the  ``.mabe`` file. 

If you don't have the ``.mabe`` file open, simply open it in your favorite text editor or IDE. CHANGE THIS!!!

From there, modifying your ``.mabe`` file is as easy as changing the values associated with the specific variable(s) that
you want to change. You can even add new variables, as long as they only rely on information you are accessing from the ``.mabe`` file
you're editing! However, if you want a new variable that takes new data inputs, then you will need to modify the specific module 
that is associated with gathering that data. You can learn more about the different modules and what they do by reading the documentation for each
module, located under the modules tab. 

To run your modified ``.mabe`` file, first make sure you have saved your file, then simply run ``./MABE -f [filename].mabe``.

Viewing and Saving Your Data
--------

The data you have collected has been saved in a CSV file called ``output.csv``, which is located in the ``build`` directory. 
From the main ``MABE2`` folder, you can find this file by running ``cd build`` and then ``ls``. To open the file, you can do so from the terminal, 
or navigate to the same ``build`` folder from your file manager and open the file from there. 

Every time that you run your ``.mabe`` file, ``output.csv`` is overwritten, which means that it is important that if you wnat to save your data, you do so between 
runs. There are a couple of ways to save your data. 

Copy the CSV File
*****************

The first way to save your data is to create a copy of ``output.csv`` (which can be done by through your file manager). 
Since there is a copy of the CSV file, you can run your ``.mabe`` file again and not worry about losing your data. 

Create a New CSV File from ``.mabe``
***********************************

The second way to save your data is to modify the ``.mabe`` file so that it saves in a different place. 

To do so, first open the ``.mabe`` file in question. TODO: figure this out. 

Within the ``.mabe`` file, there is a section called ``FileOutput``, which looks like this: 

.. code-block:: cpp

    FileOutput output {             // Output collected data into a specified file.
    _active = 1;                  // Should we activate this module? (0=off, 1=on)
    _desc = "";                   // Special description for those object.
    filename = "output.csv";      // Name of file for output data.
    format = "fitness:max,fitness:mean";// Column format to use in the file.
    target = "main_pop";          // Which population(s) should we print from?
    output_updates = "0:1";       // Which updates should we output data?
    }

Locate the variable ``filename``. Notice that right now it is labled ``"output.csv"``. 
You can modify this name to be something new, and when you run the ``.mabe`` file, a new CSV file 
with that name will appear in the same directory as the original ``output.csv`` file did. Below is an example 
of a new CSV filename inserted called ``NEW_FILE_NAME``. 

.. code-block:: cpp

    FileOutput output {             // Output collected data into a specified file.
    _active = 1;                  // Should we activate this module? (0=off, 1=on)
    _desc = "";                   // Special description for those object.
    filename = "NEW_FILE_NAME.csv";      // Name of file for output data.
    format = "fitness:max,fitness:mean";// Column format to use in the file.
    target = "main_pop";          // Which population(s) should we print from?
    output_updates = "0:1";       // Which updates should we output data?
    }

Since the ``.mabe`` file is now saving to ``NEW_FILE_NAME.csv``, the original data in ``output.csv`` is unchanged. 


