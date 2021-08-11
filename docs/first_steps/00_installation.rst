============
Installation
============

Installing Git
==============

Before attempting to install MABE2 **you must have Git installed so that you 
can use the MABE2 software on your local computer**. For information on installing 
Git on any device, see `this guide <https://github.com/git-guides/install-git#:~:text=Git%20packages%20are%20available%20using%20dnf%20.,installation%20by%20typing%3A%20git%20version%20.>`__.
For more information about GitHub, checkout the `GitHub Guides <https://guides.github.com/>`__.

Downloading MABE2 from GitHub
==============================

The first step for installation on any machine is to visit MABE2 on  
`GitHub <https://github.com/mercere99/MABE2>`__. The simplest way to 
download source code form GitHub is to download the entire repository. 
MABE2 can be downloaded :ref:`as a zip file<zip>`, or :ref:`via the command line<url>`.

.. _zip:

Downloading MABE2 as a Zip File
-------------------------------

From the repository's main page, find the green *Code* button in the upper
right hand corner. Click this button, then click *DownLoad Zip* to save the 
full zip of everything in MABE2's master branch to your computer. 

.. image:: ../images/GitHub_Zip.png
  :alt: Github zip file download example
  :width: 600

.. _url:

Downloading MABE2 via the Command Line
--------------------------------------

From the repository's main page, find the green *Code* button in the upper
right hand corner. Click this button, then click the *clipboard icon* to copy the URL to your 
clip board. 

.. image:: ../images/GitHub_url.png
  :alt: Github HTML file download example
  :width: 600

*Note*: You can also use SHH keys to clone and download a GitHub repository. 
For more information about SSH keys, checkout `this guide <https://docs.github.com/en/github/authenticating-to-github/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent>`__.

Once you have the URL copied to your clipboard, open your command line. 

Change your current working directory to the location where you want the cloned directory. 
Type `git clone`, then paste the URL into your command line.

.. code-block::

  $ git clone https://github.com/mercere99/MABE2.git

.. 

Then press enter to create your local clone! If all goes well, you will see the following
in your terminal:

.. code-block::

  $ git clone https://github.com/mercere99/MABE2.git
  > Cloning into `MABE2`...
  > remote: Counting objects: 10, done.
  > remote: Compressing objects: 100% (8/8), done.
  > remove: Total 10 (delta 1), reused 10 (delta 1)
  > Unpacking objects: 100% (10/10), done.

..

If you have issues cloning the repository, checkout `this guide <https://docs.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository-from-github/cloning-a-repository#troubleshooting-cloning-errors>`__.

Necessary compilers
===================

Once you have MABE2 downloaded you must make sure you have the necessary compilers
installed so you can run your MABE2 projects. 

To compile MABE2 **you must have a native C++ compiler**, particularly one for 
C++17 or above.

Mac and Linux
-------------

For Unix (e.g. Linux and MacOS) the most commonly used compilers are GCC and Clang.
You can check if you have GCC or Clang installed by opening your terminal and entering:

.. code-block::

  $ which gcc
  $ which clang

.. 

If a path is returned then you have gcc or clang, respectively. To check the version
enter:

.. code-block::

  $ gcc --version
  $ clang --version

..

To support C++17 you will need GCC version 5 or later or Clang version 5 or later. 

If a path is not returned then you must install gcc.

For Linux, your package manager (e.g. yum, apt, etc) will allow you to do this. 
Enter the following into your terminal to install gcc;

.. code-block::

  $ apt-get install gcc-8

.. 

For MacOS, you will need `Apple's Command Line Tools for XCode <https://developer.apple.com/xcode/features/>`__. 
To install a recent release of gcc, you can use `Homebrew <https://brew.sh/>`__ with 
`this formula <https://formulae.brew.sh/formula/gcc>`__. 

Windows
-------

The Windows Subsystem for Linux (WSL) makes it easy to run a GNU/Linux environment 
directly on Windows. For information on installing WSL, checkout `this guide <https://docs.microsoft.com/en-us/windows/wsl/install-win10>`__.
Once WSL is installed you can follow the same instructions as above. 

Next Steps
==========

Once you have downloaded MABE2 and have a C++17 compiler you can begin using MABE2!
Visit the `Quickstart guide <01_quickstart.html>`_ for detailed instructions about 
using MABE2 to run experiments. 
