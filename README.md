Protocol Buffers Extension - Google's data interchange format
==========================================================

Copyright 2008 Google Inc.

Overview
--------
This repository contains Java Protocol Buffers Nano compiler plugin and
runtime library. Google released the Java Nano for a short time, and then
decided to stop supporting it, and removed the code. I copied the removed
Javanano code here, and made it compile again. This repository contains
two following components.

Java Nano Runtime Library
-------------------------

It should be build with Maven. To generate .jar file, cd into "javanano"
and run:

     $ mvn package

To Install it into your Maven repository:

     $ mvn install

See the javanano README for more details.

Proto Compiler Plugin
-----------------------

Allows compiling the the .proto file into Java nano. To build, run:

     $ bazel build ...

You would need to have Bazel installed, see: https://bazel.build/

The resulting binary will be:

    bazel-bin/protoc-gen-javanano

Place it into a directory included in your PATH, and run:


    protoc --javanano_out=<dir> <NormalProtoParams>

