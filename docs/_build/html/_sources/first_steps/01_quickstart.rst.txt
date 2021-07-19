==========
Quickstart
==========

The ``.mabe`` File
--------

The ``.mabe`` file is generated from your chosen set of modules, and is the configuration page
that you can use to run your experiments! In the ``.mabe`` file, you can change variables that
have been set up in the different modules connected to the ``.mabe`` file. 

To generate your ``.mabe`` file, first you will want to make sure that you have run the ``make`` command 
since your last updates. To do so, navigate to the ``build`` directory and set up a clean run. From the ``MABE2`` directory, run the following commands: 

.. code-block:: cpp

    cd build
    make clean ; make
    
If there are any errors that pop up, now is the time to fix them!

Next, you will navigate to the ``settings`` directory inside of ``build``, and check to see you have an appropriate ``.gen`` file. To do so, first make sure you 
are in the ``build`` directory, and then list all of the files in the settings to make sure the ``.gen`` file is there. 

.. code-block:: cpp

    cd settings
    ls

From here, you'll want to make sure you can see your .gen file. You should see a file labled ``<example_filename>.gen``. If not, you can  
follow the directions to `write your .gen file <000_write_gen_file.html>`_. 


Now we're ready to create our ``.mabe`` file! To do so, we'll navigate back up to ``build`` and then create the ``.mabe`` file. You'll run the
following commands to do so: 

.. code-block:: cpp

    cd ..
    ./MABE -f settings/<example_filename>.gen -g settings/<example_filename>.mabe


This will generate a ``.mabe`` file named ``<example_filename>.mabe`` with the specifications from the ``<example_filename>.gen`` file. 
You can check to see that it exists by going into the ``settings`` directory and checking that it's there. To do so, run the following: 

.. code-block:: cpp

    cd settings
    ls

Congratulations! You've created your first ``.mabe`` file!

Summary
*****
Step 1: In the ``build`` directory, run the following: 

.. code-block:: cpp

    make clean ; make


Step 2: Then run these commands to make sure your ``.gen`` file exists. 

.. code-block:: cpp

    cd settings
    ls 

If you don't see it, you can create a ``.gen`` file by following the steps 
to `write your .gen file <000_write_gen_file.html>`_. 

Step 3: Create your ``.mabe`` file and check to make sure it's created by running the following: 

.. code-block:: cpp

    cd ..
    ./MABE -f settings/<example_filename>.gen -g settings/<example_filename>.mabe
    cd settings
    ls


Running the ``.mabe`` File
--------

To run your ``.mabe`` file, navigate to the ``build`` directory and run your ``.mabe`` file. To do so, start in the ``MABE2`` folder and run the following commands: 

.. code-block:: cpp 

    cd build
    ./MABE -f settings/<example_filename>.mabe
    

Changing the ``.mabe`` File
--------
You can modify your experiment by changing the variable values inside of the  ``.mabe`` file. 

If you don't have the ``.mabe`` file open, simply open it in your text editor of choice. 

From there, modifying your ``.mabe`` file is as easy as changing the values associated with the specific variable(s) that
you want to change. You can even add new variables, as long as they only rely on information you are accessing from the ``.mabe`` file
you're editing! However, if you want a new variable that takes new data inputs, then you will need to modify the specific module 
that is associated with gathering that data. You can learn more about the different modules and what they do by reading the documentation for each
module, located in the `Modules Page <../modules/00_module_overview.html>`_ . 

To run your modified ``.mabe`` file, first make sure you have saved your file, then simply run the following command from the ``build`` directory: 

.. code-block:: cpp
    ./MABE -f settings/<example_filename>.mabe



Viewing and Saving Your Data
--------

The data you have collected has been saved in a CSV file called ``output.csv``, which is located in the ``build`` directory. 
From the main ``MABE2`` folder, you can find this file by running the following commands: 

.. code-block:: cpp

    cd build
    ls
    
To open the file, you can do so from the terminal, 
or navigate to the same ``build`` folder from your file manager and open the file from there. 

Every time that you run your ``.mabe`` file, ``output.csv`` is overwritten, which means that it is important that if you want to save your data, you do so between 
runs. There are a couple of ways to save your data. 

Copy the CSV File
*****************

The first way to save your data is to create a copy of ``output.csv`` (which can be done by through your file manager). 
Since there is a copy of the CSV file, you can run your ``.mabe`` file again and not worry about losing your data. 

Create a New CSV File from ``.mabe``
***********************************

The second way to save your data is to modify the ``.mabe`` file itself so that it saves in a different place. 

To do so, first open the ``.mabe`` file in question in your preferred text editor. It will be in the ``settings`` folder inside of ``build``. 

Within the ``.mabe`` file, there is a section called ``FileOutput``, which looks something like this: 

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
with that name will appear in the same directory as the original ``output.csv`` file. Below is an example 
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


