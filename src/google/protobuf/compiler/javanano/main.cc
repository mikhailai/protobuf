#include <iostream>
#include <string>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/javanano/javanano_generator.h>

static const char USAGE[] =
"Protocol Buffer Compiler Plugin for generating Javanano output.\n"
"You should not invoke this directly. Instead, place this executable in a \n"
"directory in a PATH, and then run:\n"
"   protoc --javanano_out=<dir> <other options>\n"
"\n"
"This will generate a Javanano output.\n";

int main(int argc, char* argv[]) {
    {
      std::string help1("--help");
      std::string help2("-h");
      if (argc == 2 && (help1 == argv[1] || help2 == argv[1])) {
        std::cerr << argv[0] << ":" << std::endl << USAGE;
        return 1;
      }
    }

    google::protobuf::compiler::javanano::JavaNanoGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
