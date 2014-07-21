// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include <limits>
#include <vector>

#include <google/protobuf/compiler/javanano/javanano_helpers.h>
#include <google/protobuf/compiler/javanano/javanano_params.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/stubs/hash.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace javanano {

const char kThickSeparator[] =
  "// ===================================================================\n";
const char kThinSeparator[] =
  "// -------------------------------------------------------------------\n";

class RenameKeywords {
 private:
  hash_set<string> java_keywords_set_;

 public:
  RenameKeywords() {
    static const char* kJavaKeywordsList[] = {
      // Reserved Java Keywords
      "abstract", "assert", "boolean", "break", "byte", "case", "catch",
      "char", "class", "const", "continue", "default", "do", "double", "else",
      "enum", "extends", "final", "finally", "float", "for", "goto", "if",
      "implements", "import", "instanceof", "int", "interface", "long",
      "native", "new", "package", "private", "protected", "public", "return",
      "short", "static", "strictfp", "super", "switch", "synchronized",
      "this", "throw", "throws", "transient", "try", "void", "volatile", "while",

      // Reserved Keywords for Literals
      "false", "null", "true"
    };

    for (int i = 0; i < GOOGLE_ARRAYSIZE(kJavaKeywordsList); i++) {
      java_keywords_set_.insert(kJavaKeywordsList[i]);
    }
  }

  // Used to rename the a field name if it's a java keyword.  Specifically
  // this is used to rename the ["name"] or ["capitalized_name"] field params.
  // (http://docs.oracle.com/javase/tutorial/java/nutsandbolts/_keywords.html)
  string RenameJavaKeywordsImpl(const string& input) {
    string result = input;

    if (java_keywords_set_.find(result) != java_keywords_set_.end()) {
      result += "_";
    }

    return result;
  }

};

static RenameKeywords sRenameKeywords;

namespace {

const char* kDefaultPackage = "";

const string& FieldName(const FieldDescriptor* field) {
  // Groups are hacky:  The name of the field is just the lower-cased name
  // of the group type.  In Java, though, we would like to retain the original
  // capitalization of the type name.
  if (field->type() == FieldDescriptor::TYPE_GROUP) {
    return field->message_type()->name();
  } else {
    return field->name();
  }
}

string UnderscoresToCamelCaseImpl(const string& input, bool cap_next_letter) {
  string result;
  // Note:  I distrust ctype.h due to locales.
  for (int i = 0; i < input.size(); i++) {
    if ('a' <= input[i] && input[i] <= 'z') {
      if (cap_next_letter) {
        result += input[i] + ('A' - 'a');
      } else {
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('A' <= input[i] && input[i] <= 'Z') {
      if (i == 0 && !cap_next_letter) {
        // Force first letter to lower-case unless explicitly told to
        // capitalize it.
        result += input[i] + ('a' - 'A');
      } else {
        // Capital letters after the first are left as-is.
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('0' <= input[i] && input[i] <= '9') {
      result += input[i];
      cap_next_letter = true;
    } else {
      cap_next_letter = true;
    }
  }
  return result;
}

}  // namespace

string UnderscoresToCamelCase(const FieldDescriptor* field) {
  return UnderscoresToCamelCaseImpl(FieldName(field), false);
}

string UnderscoresToCapitalizedCamelCase(const FieldDescriptor* field) {
  return UnderscoresToCamelCaseImpl(FieldName(field), true);
}

string UnderscoresToCamelCase(const MethodDescriptor* method) {
  return UnderscoresToCamelCaseImpl(method->name(), false);
}

string RenameJavaKeywords(const string& input) {
  return sRenameKeywords.RenameJavaKeywordsImpl(input);
}

string StripProto(const string& filename) {
  if (HasSuffixString(filename, ".protodevel")) {
    return StripSuffixString(filename, ".protodevel");
  } else {
    return StripSuffixString(filename, ".proto");
  }
}

string FileClassName(const Params& params, const FileDescriptor* file) {
  if (params.has_java_outer_classname(file->name())) {
    return params.java_outer_classname(file->name());
  } else {
    // Use the filename itself with underscores removed
    // and a CamelCase style name.
    string basename;
    string::size_type last_slash = file->name().find_last_of('/');
    if (last_slash == string::npos) {
      basename = file->name();
    } else {
      basename = file->name().substr(last_slash + 1);
    }
    return UnderscoresToCamelCaseImpl(StripProto(basename), true);
  }
}

string FileJavaPackage(const Params& params, const FileDescriptor* file) {
  if (params.has_java_package(file->name())) {
    return params.java_package(file->name());
  } else {
    string result = kDefaultPackage;
    if (!file->package().empty()) {
      if (!result.empty()) result += '.';
      result += file->package();
    }
    return result;
  }
}

bool IsOuterClassNeeded(const Params& params, const FileDescriptor* file) {
  // If java_multiple_files is false, the outer class is always needed.
  if (!params.java_multiple_files(file->name())) {
    return true;
  }

  // File-scope extensions need the outer class as the scope.
  if (file->extension_count() != 0) {
    return true;
  }

  // If container interfaces are not generated, file-scope enums need the
  // outer class as the scope.
  if (file->enum_type_count() != 0 && !params.java_enum_style()) {
    return true;
  }

  return false;
}

string ToJavaName(const Params& params, const string& name, bool is_class,
    const Descriptor* parent, const FileDescriptor* file) {
  string result;
  if (parent != NULL) {
    result.append(ClassName(params, parent));
  } else if (is_class && params.java_multiple_files(file->name())) {
    result.append(FileJavaPackage(params, file));
  } else {
    result.append(ClassName(params, file));
  }
  if (!result.empty()) result.append(1, '.');
  result.append(RenameJavaKeywords(name));
  return result;
}

string ClassName(const Params& params, const FileDescriptor* descriptor) {
  string result = FileJavaPackage(params, descriptor);
  if (!result.empty()) result += '.';
  result += FileClassName(params, descriptor);
  return result;
}

string ClassName(const Params& params, const EnumDescriptor* descriptor) {
  const Descriptor* parent = descriptor->containing_type();
  // When using Java enum style, an enum's class name contains the enum name.
  // Use the standard ToJavaName translation.
  if (params.java_enum_style()) {
    return ToJavaName(params, descriptor->name(), true, parent,
                      descriptor->file());
  }
  // Otherwise the enum members are accessed from the enclosing class.
  if (parent != NULL) {
    return ClassName(params, parent);
  } else {
    return ClassName(params, descriptor->file());
  }
}

string FieldConstantName(const FieldDescriptor *field) {
  string name = field->name() + "_FIELD_NUMBER";
  UpperString(&name);
  return name;
}

string FieldDefaultConstantName(const FieldDescriptor *field) {
  return "_" + RenameJavaKeywords(UnderscoresToCamelCase(field)) + "Default";
}

void PrintFieldComment(io::Printer* printer, const FieldDescriptor* field) {
  // We don't want to print group bodies so we cut off after the first line
  // (the second line for extensions).
  string def = field->DebugString();
  string::size_type first_line_end = def.find_first_of('\n');
  printer->Print("// $def$\n",
    "def", def.substr(0, first_line_end));
  if (field->is_extension()) {
    string::size_type second_line_start = first_line_end + 1;
    string::size_type second_line_length =
        def.find('\n', second_line_start) - second_line_start;
    printer->Print("// $def$\n",
      "def", def.substr(second_line_start, second_line_length));
  }
}

JavaType GetJavaType(FieldDescriptor::Type field_type) {
  switch (field_type) {
    case FieldDescriptor::TYPE_INT32:
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_SFIXED32:
      return JAVATYPE_INT;

    case FieldDescriptor::TYPE_INT64:
    case FieldDescriptor::TYPE_UINT64:
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_SFIXED64:
      return JAVATYPE_LONG;

    case FieldDescriptor::TYPE_FLOAT:
      return JAVATYPE_FLOAT;

    case FieldDescriptor::TYPE_DOUBLE:
      return JAVATYPE_DOUBLE;

    case FieldDescriptor::TYPE_BOOL:
      return JAVATYPE_BOOLEAN;

    case FieldDescriptor::TYPE_STRING:
      return JAVATYPE_STRING;

    case FieldDescriptor::TYPE_BYTES:
      return JAVATYPE_BYTES;

    case FieldDescriptor::TYPE_ENUM:
      return JAVATYPE_ENUM;

    case FieldDescriptor::TYPE_GROUP:
    case FieldDescriptor::TYPE_MESSAGE:
      return JAVATYPE_MESSAGE;

    // No default because we want the compiler to complain if any new
    // types are added.
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return JAVATYPE_INT;
}

string PrimitiveTypeName(JavaType type) {
  switch (type) {
    case JAVATYPE_INT    : return "int";
    case JAVATYPE_LONG   : return "long";
    case JAVATYPE_FLOAT  : return "float";
    case JAVATYPE_DOUBLE : return "double";
    case JAVATYPE_BOOLEAN: return "boolean";
    case JAVATYPE_STRING : return "java.lang.String";
    case JAVATYPE_BYTES  : return "byte[]";
    case JAVATYPE_ENUM   : return "int";
    case JAVATYPE_MESSAGE: return "";

    // No default because we want the compiler to complain if any new
    // JavaTypes are added.
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}

string BoxedPrimitiveTypeName(JavaType type) {
  switch (type) {
    case JAVATYPE_INT    : return "java.lang.Integer";
    case JAVATYPE_LONG   : return "java.lang.Long";
    case JAVATYPE_FLOAT  : return "java.lang.Float";
    case JAVATYPE_DOUBLE : return "java.lang.Double";
    case JAVATYPE_BOOLEAN: return "java.lang.Boolean";
    case JAVATYPE_STRING : return "java.lang.String";
    case JAVATYPE_BYTES  : return "byte[]";
    case JAVATYPE_ENUM   : return "java.lang.Integer";
    case JAVATYPE_MESSAGE: return "";

    // No default because we want the compiler to complain if any new
    // JavaTypes are added.
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}

string EmptyArrayName(const Params& params, const FieldDescriptor* field) {
  switch (GetJavaType(field)) {
    case JAVATYPE_INT    : return "com.google.protobuf.nano.WireFormatNano.EMPTY_INT_ARRAY";
    case JAVATYPE_LONG   : return "com.google.protobuf.nano.WireFormatNano.EMPTY_LONG_ARRAY";
    case JAVATYPE_FLOAT  : return "com.google.protobuf.nano.WireFormatNano.EMPTY_FLOAT_ARRAY";
    case JAVATYPE_DOUBLE : return "com.google.protobuf.nano.WireFormatNano.EMPTY_DOUBLE_ARRAY";
    case JAVATYPE_BOOLEAN: return "com.google.protobuf.nano.WireFormatNano.EMPTY_BOOLEAN_ARRAY";
    case JAVATYPE_STRING : return "com.google.protobuf.nano.WireFormatNano.EMPTY_STRING_ARRAY";
    case JAVATYPE_BYTES  : return "com.google.protobuf.nano.WireFormatNano.EMPTY_BYTES_ARRAY";
    case JAVATYPE_ENUM   : return "com.google.protobuf.nano.WireFormatNano.EMPTY_INT_ARRAY";
    case JAVATYPE_MESSAGE: return ClassName(params, field->message_type()) + ".EMPTY_ARRAY";

    // No default because we want the compiler to complain if any new
    // JavaTypes are added.
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}

string DefaultValue(const Params& params, const FieldDescriptor* field) {
  if (field->label() == FieldDescriptor::LABEL_REPEATED) {
    return EmptyArrayName(params, field);
  }

  if (params.use_reference_types_for_primitives()) {
    return "null";
  }

  // Switch on cpp_type since we need to know which default_value_* method
  // of FieldDescriptor to call.
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return SimpleItoa(field->default_value_int32());
    case FieldDescriptor::CPPTYPE_UINT32:
      // Need to print as a signed int since Java has no unsigned.
      return SimpleItoa(static_cast<int32>(field->default_value_uint32()));
    case FieldDescriptor::CPPTYPE_INT64:
      return SimpleItoa(field->default_value_int64()) + "L";
    case FieldDescriptor::CPPTYPE_UINT64:
      return SimpleItoa(static_cast<int64>(field->default_value_uint64())) +
             "L";
    case FieldDescriptor::CPPTYPE_DOUBLE: {
      double value = field->default_value_double();
      if (value == numeric_limits<double>::infinity()) {
        return "Double.POSITIVE_INFINITY";
      } else if (value == -numeric_limits<double>::infinity()) {
        return "Double.NEGATIVE_INFINITY";
      } else if (value != value) {
        return "Double.NaN";
      } else {
        return SimpleDtoa(value) + "D";
      }
    }
    case FieldDescriptor::CPPTYPE_FLOAT: {
      float value = field->default_value_float();
      if (value == numeric_limits<float>::infinity()) {
        return "Float.POSITIVE_INFINITY";
      } else if (value == -numeric_limits<float>::infinity()) {
        return "Float.NEGATIVE_INFINITY";
      } else if (value != value) {
        return "Float.NaN";
      } else {
        return SimpleFtoa(value) + "F";
      }
    }
    case FieldDescriptor::CPPTYPE_BOOL:
      return field->default_value_bool() ? "true" : "false";
    case FieldDescriptor::CPPTYPE_STRING:
      if (!field->default_value_string().empty()) {
        // Point it to the static final in the generated code.
        return FieldDefaultConstantName(field);
      } else {
        if (field->type() == FieldDescriptor::TYPE_BYTES) {
          return "com.google.protobuf.nano.WireFormatNano.EMPTY_BYTES";
        } else {
          return "\"\"";
        }
      }

    case FieldDescriptor::CPPTYPE_ENUM:
      return ClassName(params, field->enum_type()) + "." +
             RenameJavaKeywords(field->default_value_enum()->name());

    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "null";

    // No default because we want the compiler to complain if any new
    // types are added.
  }

  GOOGLE_LOG(FATAL) << "Can't get here.";
  return "";
}


static const char* kBitMasks[] = {
  "0x00000001",
  "0x00000002",
  "0x00000004",
  "0x00000008",
  "0x00000010",
  "0x00000020",
  "0x00000040",
  "0x00000080",

  "0x00000100",
  "0x00000200",
  "0x00000400",
  "0x00000800",
  "0x00001000",
  "0x00002000",
  "0x00004000",
  "0x00008000",

  "0x00010000",
  "0x00020000",
  "0x00040000",
  "0x00080000",
  "0x00100000",
  "0x00200000",
  "0x00400000",
  "0x00800000",

  "0x01000000",
  "0x02000000",
  "0x04000000",
  "0x08000000",
  "0x10000000",
  "0x20000000",
  "0x40000000",
  "0x80000000",
};

string GetBitFieldName(int index) {
  string var_name = "bitField";
  var_name += SimpleItoa(index);
  var_name += "_";
  return var_name;
}

string GetBitFieldNameForBit(int bit_index) {
  return GetBitFieldName(bit_index / 32);
}

string GenerateGetBit(int bit_index) {
  string var_name = GetBitFieldNameForBit(bit_index);
  int bit_in_var_index = bit_index % 32;

  string mask = kBitMasks[bit_in_var_index];
  string result = "((" + var_name + " & " + mask + ") != 0)";
  return result;
}

string GenerateSetBit(int bit_index) {
  string var_name = GetBitFieldNameForBit(bit_index);
  int bit_in_var_index = bit_index % 32;

  string mask = kBitMasks[bit_in_var_index];
  string result = var_name + " |= " + mask;
  return result;
}

string GenerateClearBit(int bit_index) {
  string var_name = GetBitFieldNameForBit(bit_index);
  int bit_in_var_index = bit_index % 32;

  string mask = kBitMasks[bit_in_var_index];
  string result = var_name + " = (" + var_name + " & ~" + mask + ")";
  return result;
}

string GenerateDifferentBit(int bit_index) {
  string var_name = GetBitFieldNameForBit(bit_index);
  int bit_in_var_index = bit_index % 32;

  string mask = kBitMasks[bit_in_var_index];
  string result = "((" + var_name + " & " + mask
      + ") != (other." + var_name + " & " + mask + "))";
  return result;
}

void SetBitOperationVariables(const string name,
    int bitIndex, map<string, string>* variables) {
  (*variables)["get_" + name] = GenerateGetBit(bitIndex);
  (*variables)["set_" + name] = GenerateSetBit(bitIndex);
  (*variables)["clear_" + name] = GenerateClearBit(bitIndex);
  (*variables)["different_" + name] = GenerateDifferentBit(bitIndex);
}

}  // namespace javanano
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
