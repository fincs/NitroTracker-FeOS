NitroTracker for FeOS v0.0 (WIP)
================================

Copyright (c) Tobias "0xtob" Weyand (original program), fincs (FeOS port)

Official NitroTracker website: http://nitrotracker.tobw.net/

Introduction
------------

NitroTracker is a very popular FastTracker II style tracker for the Nintendo DS. This is a port to FeOS in order to demonstrate the capabilities of the FeOS environment.

How to build
------------

You need the following in order to build NitroTracker for FeOS:

- Latest [FeOS and FeOS SDK](https://github.com/fincs/FeOS)
- [μSTL for FeOS](http://feos.mtheall.com/uSTL-FeOS/)
- A brain

On the main repository directory, type the following command:

    make install

This will automatically build and copy all components to the FeOS target folder.

Basic usage
-----------

In order to launch NitroTracker, type this command into the FeOS command prompt:

    ntracker

You may directly load a XM file via adding a parameter:

    ntracker yourFile.xm

Differences from the official NitroTracker
------------------------------------------

* The splash screen was removed
* An 'Exit app' button was added to the about dialog box (click the NitroTracker icon)
* A 'New' button was added to the file loading tab
* DSMidiWifi support has been temporarily disabled

To do
-----

- Replace DSMidiWifi with something else
- Finish fixing a nasty memory leak
- Continue writing the "To do" section
