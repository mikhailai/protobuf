Extension for Google's Protocol Buffers
========================================

Copyright 2008 Google Inc.<br>
The protocol buffers code:
https://github.com/protocolbuffers/protobuf<br>
Reference:
https://developers.google.com/protocol-buffers/

Overview
--------
This repository contains Java Protocol Buffers Nano compiler plugin and
runtime library. Google published the Java Nano code for a short time, 
then stoped supporting it, and removed the code. The repository contains
that code, made compile again. It consists of the two following components.

Java Nano Runtime Library
-------------------------

Should be build with Maven. To generate .jar file, cd into "javanano"
and run:

     $ mvn package

To Install it into your Maven repository:

     $ mvn install

See the javanano README for more details.

Proto Compiler Plugin
-----------------------

Allows compiling the the .proto file into Java nano. To build, run:

     $ bazel build ...

You would need to install Bazel, see https://bazel.build for more details.

The resulting binary will be:

    bazel-bin/protoc-gen-javanano

Place it into a directory included in your PATH, and run:


    protoc --javanano_out=<dir> <NormalProtoParams>

