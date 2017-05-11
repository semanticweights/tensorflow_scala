/* Copyright 2016 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "include/tensor_jni.h"

#include <algorithm>
//#include <assert.h>
//#include <memory>
//#include <stdlib.h>
//#include <string.h>
//#include <utility>

#include "include/c_api.h"
#include "include/exception_jni.h"

namespace {
  TF_Tensor* require_graph_handle(JNIEnv *env, jlong handle) {
    static_assert(sizeof(jlong) >= sizeof(TF_Tensor*),
                  "Scala \"Long\" cannot be used to represent TensorFlow C API pointers.");
    if (handle == 0) {
      throw_exception(env, jvm_null_pointer_exception, "This tensor has already been disposed.");
      return nullptr;
    }
    return reinterpret_cast<TF_Tensor*>(handle);
  }
}  // namespace

JNIEXPORT jlong JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_fromBuffer(
    JNIEnv* env, jobject object, jobject buffer, jint dtype, jlongArray shape, jlong sizeInBytes) {
  int num_dims = static_cast<int>(env->GetArrayLength(shape));
  jlong* dims = nullptr;
  if (num_dims > 0) {
    jboolean is_copy;
    dims = env->GetLongArrayElements(shape, &is_copy);
  }
  static_assert(sizeof(jlong) == sizeof(int64_t), "Scala \"Long\" is not compatible with the TensorFlow C API.");
  // On some platforms "jlong" is a "long" while "int64_t" is a "long long".
  //
  // Thus, static_cast<int64_t*>(dims) will trigger a compiler error:
  // static_cast from 'jlong *' (aka 'long *') to 'int64_t *' (aka 'long long
  // *') is not allowed
  //
  // Since this array is typically very small, use the guaranteed safe scheme of
  // creating a copy.
  int64_t* dims_copy = new int64_t[num_dims];
  for (int i = 0; i < num_dims; ++i)
    dims_copy[i] = static_cast<int64_t>(dims[i]);
  // Notifying the JVM of the existence of this reference to the byte buffer, to avoid garbage collection.
  // More details can be found here: http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/design.html#wp1242
  jobject buffer_ref = env->NewGlobalRef(buffer);
  std::pair<JNIEnv*, jobject>* deallocator_arg = new std::pair<JNIEnv*, jobject>(env, buffer_ref);
  TF_Tensor* tensor_handle = TF_NewTensor(
      static_cast<TF_DataType>(dtype), dims_copy, num_dims,
      env->GetDirectBufferAddress(buffer), static_cast<size_t>(sizeInBytes),
      [](void* data, size_t len, void* arg) {
        std::pair<JNIEnv*, jobject>* pair = reinterpret_cast<std::pair<JNIEnv*, jobject>*>(arg);
        pair->first->DeleteGlobalRef(reinterpret_cast<jobject>(pair->second));
        delete (pair);
      }, deallocator_arg);
  delete[] dims_copy;
  if (dims != nullptr)
    env->ReleaseLongArrayElements(shape, dims, JNI_ABORT);
  if (tensor_handle == nullptr) {
    throw_exception(env, jvm_null_pointer_exception, "Unable to create new native Tensor.");
    return 0;
  }
  return reinterpret_cast<jlong>(tensor_handle);
}

JNIEXPORT jint JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_dataType(
    JNIEnv* env, jobject object, jlong handle) {
  static_assert(sizeof(jint) >= sizeof(TF_DataType),
                "\"TF_DataType\" in C cannot be represented as an \"Int\" in Scala.");
  TF_Tensor* tensor = require_graph_handle(env, handle);
  if (tensor == nullptr) return 0;
  return static_cast<jint>(TF_TensorType(tensor));
}

JNIEXPORT jlongArray JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_shape(
    JNIEnv* env, jobject object, jlong handle) {
  TF_Tensor* tensor = require_graph_handle(env, handle);
  if (tensor == nullptr) return nullptr;
  static_assert(sizeof(jlong) == sizeof(int64_t), "Scala \"Long\" is not compatible with the TensorFlow C API.");
  const jsize num_dims = TF_NumDims(tensor);
  jlongArray return_array = env->NewLongArray(num_dims);
  jlong* shape = env->GetLongArrayElements(return_array, nullptr);
  for (int i = 0; i < num_dims; ++i)
    shape[i] = static_cast<jlong>(TF_Dim(tensor, i));
  env->ReleaseLongArrayElements(return_array, shape, 0);
  return return_array;
}

JNIEXPORT jobject JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_buffer(
    JNIEnv* env, jobject object, jlong handle) {
  TF_Tensor* tensor = require_graph_handle(env, handle);
  if (tensor == nullptr) return nullptr;
  void* data = TF_TensorData(tensor);
  const size_t byte_size = TF_TensorByteSize(tensor);
  return env->NewDirectByteBuffer(data, static_cast<jlong>(byte_size));
}

JNIEXPORT void JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_delete(
    JNIEnv* env, jobject object, jlong handle) {
  if (handle == 0) return;
  TF_DeleteTensor(reinterpret_cast<TF_Tensor*>(handle));
}

JNIEXPORT jint JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_getEncodedStringSize(
    JNIEnv* env, jobject object, jint string_num_bytes) {
  return static_cast<jint>(TF_StringEncodedSize(static_cast<size_t>(string_num_bytes)));
}

JNIEXPORT jint JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_setStringBytes(
    JNIEnv* env, jobject object, jbyteArray string, jobject string_buffer) {
  char* dst_buffer = reinterpret_cast<char*>(env->GetDirectBufferAddress(string_buffer));
  size_t src_len = static_cast<size_t>(env->GetArrayLength(string));
  size_t dst_len = TF_StringEncodedSize(src_len);
  std::unique_ptr<char[]> buffer(new char[src_len]);
  env->GetByteArrayRegion(string, 0, static_cast<jsize>(src_len), reinterpret_cast<jbyte*>(buffer.get()));
  TF_Status* status = TF_NewStatus();
  size_t num_bytes_written = TF_StringEncode(buffer.get(), src_len, dst_buffer, dst_len, status);
  throw_exception_if_not_ok(env, status);
  TF_DeleteStatus(status);
  return static_cast<jsize>(num_bytes_written);
}

JNIEXPORT jbyteArray JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_getStringBytes(
    JNIEnv* env, jobject object, jobject string_buffer) {
  char* src_buffer = reinterpret_cast<char*>(env->GetDirectBufferAddress(string_buffer));
  jbyteArray return_array = nullptr;
  const char* dst = nullptr;
  size_t dst_len = 0;
  TF_Status* status = TF_NewStatus();
  size_t src_len = static_cast<size_t>(env->GetDirectBufferCapacity(string_buffer));
  TF_StringDecode(src_buffer, src_len, &dst, &dst_len, status);
  if (throw_exception_if_not_ok(env, status)) {
    return_array = env->NewByteArray(static_cast<jsize>(dst_len));
    env->SetByteArrayRegion(return_array, 0, static_cast<jsize>(dst_len), reinterpret_cast<const jbyte*>(dst));
  }
  TF_DeleteStatus(status);
  return return_array;
}

//namespace {
//
//TF_Tensor* requireHandle(JNIEnv* env, jlong handle) {
//  if (handle == 0) {
//    throw_exception(env, kNullPointerException,
//                   "close() was called on the Tensor");
//    return nullptr;
//  }
//  return reinterpret_cast<TF_Tensor*>(handle);
//}
//
//size_t elemByteSize(TF_DataType dtype) {
//  // The code in this file makes the assumption that the
//  // TensorFlow TF_DataTypes and the Java primitive types
//  // have the same byte sizes. Validate that:
//  switch (dtype) {
//    case TF_BOOL:
//      static_assert(sizeof(jboolean) == 1,
//                    "Java boolean not compatible with TF_BOOL");
//      return 1;
//    case TF_FLOAT:
//    case TF_INT32:
//      static_assert(sizeof(jfloat) == 4,
//                    "Java float not compatible with TF_FLOAT");
//      static_assert(sizeof(jint) == 4, "Java int not compatible with TF_INT32");
//      return 4;
//    case TF_DOUBLE:
//    case TF_INT64:
//      static_assert(sizeof(jdouble) == 8,
//                    "Java double not compatible with TF_DOUBLE");
//      static_assert(sizeof(jlong) == 8,
//                    "Java long not compatible with TF_INT64");
//      return 8;
//    default:
//      return 0;
//  }
//}
//
//// Write a Java scalar object (java.lang.Integer etc.) to a TF_Tensor.
//void writeScalar(JNIEnv* env, jobject src, TF_DataType dtype, void* dst,
//                 size_t dst_size) {
//  size_t sz = elemByteSize(dtype);
//  if (sz != dst_size) {
//    throw_exception(
//        env, kIllegalStateException,
//        "scalar (%d bytes) not compatible with allocated tensor (%d bytes)", sz,
//        dst_size);
//    return;
//  }
//  switch (dtype) {
//// env->FindClass and env->GetMethodID are expensive and JNI best practices
//// suggest that they should be cached. However, until the creation of scalar
//// valued tensors seems to become a noticeable fraction of program execution,
//// ignore that cost.
//#define CASE(dtype, jtype, method_name, method_signature, call_type)           \
//  case dtype: {                                                                \
//    jclass clazz = env->FindClass("java/lang/Number");                         \
//    jmethodID method = env->GetMethodID(clazz, method_name, method_signature); \
//    jtype v = env->Call##call_type##Method(src, method);                       \
//    memcpy(dst, &v, sz);                                                       \
//    return;                                                                    \
//  }
//    CASE(TF_FLOAT, jfloat, "floatValue", "()F", Float);
//    CASE(TF_DOUBLE, jdouble, "doubleValue", "()D", Double);
//    CASE(TF_INT32, jint, "intValue", "()I", Int);
//    CASE(TF_INT64, jlong, "longValue", "()J", Long);
//#undef CASE
//    case TF_BOOL: {
//      jclass clazz = env->FindClass("java/lang/Boolean");
//      jmethodID method = env->GetMethodID(clazz, "booleanValue", "()Z");
//      jboolean v = env->CallBooleanMethod(src, method);
//      *(static_cast<unsigned char*>(dst)) = v ? 1 : 0;
//      return;
//    }
//    default:
//      throw_exception(env, kIllegalStateException, "invalid DataType(%d)",
//                     dtype);
//      return;
//  }
//}
//
//// Copy a 1-D array of Java primitive types to the tensor buffer dst.
//// Returns the number of bytes written to dst.
//size_t write1DArray(JNIEnv* env, jarray array, TF_DataType dtype, void* dst,
//                    size_t dst_size) {
//  const int nelems = env->GetArrayLength(array);
//  jboolean is_copy;
//  switch (dtype) {
//#define CASE(dtype, jtype, get_type)                                   \
//  case dtype: {                                                        \
//    jtype##Array a = static_cast<jtype##Array>(array);                 \
//    jtype* values = env->Get##get_type##ArrayElements(a, &is_copy);    \
//    size_t to_copy = nelems * elemByteSize(dtype);                     \
//    if (to_copy > dst_size) {                                          \
//      throw_exception(                                                  \
//          env, kIllegalStateException,                                 \
//          "cannot write Java array of %d bytes to Tensor of %d bytes", \
//          to_copy, dst_size);                                          \
//      to_copy = 0;                                                     \
//    } else {                                                           \
//      memcpy(dst, values, to_copy);                                    \
//    }                                                                  \
//    env->Release##get_type##ArrayElements(a, values, JNI_ABORT);       \
//    return to_copy;                                                    \
//  }
//    CASE(TF_FLOAT, jfloat, Float);
//    CASE(TF_DOUBLE, jdouble, Double);
//    CASE(TF_INT32, jint, Int);
//    CASE(TF_INT64, jlong, Long);
//    CASE(TF_BOOL, jboolean, Boolean);
//#undef CASE
//    default:
//      throw_exception(env, kIllegalStateException, "invalid DataType(%d)",
//                     dtype);
//      return 0;
//  }
//}
//
//// Copy the elements of a 1-D array from the tensor buffer src to a 1-D array of
//// Java primitive types. Returns the number of bytes read from src.
//size_t read1DArray(JNIEnv* env, TF_DataType dtype, const void* src,
//                   size_t src_size, jarray dst) {
//  const int len = env->GetArrayLength(dst);
//  const size_t sz = len * elemByteSize(dtype);
//  if (sz > src_size) {
//    throw_exception(
//        env, kIllegalStateException,
//        "cannot fill a Java array of %d bytes with a Tensor of %d bytes", sz,
//        src_size);
//    return 0;
//  }
//  switch (dtype) {
//#define CASE(dtype, jtype, primitive_type)                                 \
//  case dtype: {                                                            \
//    jtype##Array arr = static_cast<jtype##Array>(dst);                     \
//    env->Set##primitive_type##ArrayRegion(arr, 0, len,                     \
//                                          static_cast<const jtype*>(src)); \
//    return sz;                                                             \
//  }
//    CASE(TF_FLOAT, jfloat, Float);
//    CASE(TF_DOUBLE, jdouble, Double);
//    CASE(TF_INT32, jint, Int);
//    CASE(TF_INT64, jlong, Long);
//    CASE(TF_BOOL, jboolean, Boolean);
//#undef CASE
//    default:
//      throw_exception(env, kIllegalStateException, "invalid DataType(%d)",
//                     dtype);
//  }
//  return 0;
//}
//
//size_t writeNDArray(JNIEnv* env, jarray src, TF_DataType dtype, int dims_left,
//                    char* dst, size_t dst_size) {
//  if (dims_left == 1) {
//    return write1DArray(env, src, dtype, dst, dst_size);
//  } else {
//    jobjectArray ndarray = static_cast<jobjectArray>(src);
//    int len = env->GetArrayLength(ndarray);
//    size_t sz = 0;
//    for (int i = 0; i < len; ++i) {
//      jarray row = static_cast<jarray>(env->GetObjectArrayElement(ndarray, i));
//      sz +=
//          writeNDArray(env, row, dtype, dims_left - 1, dst + sz, dst_size - sz);
//      env->DeleteLocalRef(row);
//      if (env->ExceptionCheck()) return sz;
//    }
//    return sz;
//  }
//}
//
//size_t readNDArray(JNIEnv* env, TF_DataType dtype, const char* src,
//                   size_t src_size, int dims_left, jarray dst) {
//  if (dims_left == 1) {
//    return read1DArray(env, dtype, src, src_size, dst);
//  } else {
//    jobjectArray ndarray = static_cast<jobjectArray>(dst);
//    int len = env->GetArrayLength(ndarray);
//    size_t sz = 0;
//    for (int i = 0; i < len; ++i) {
//      jarray row = static_cast<jarray>(env->GetObjectArrayElement(ndarray, i));
//      sz +=
//          readNDArray(env, dtype, src + sz, src_size - sz, dims_left - 1, row);
//      env->DeleteLocalRef(row);
//      if (env->ExceptionCheck()) return sz;
//    }
//    return sz;
//  }
//}
//}  // namespace

//JNIEXPORT jlong JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_allocate(JNIEnv* env,
//                                                                            jobject object,
//                                                                            jint dtype,
//                                                                            jlongArray shape,
//                                                                            jlong sizeInBytes) {
//  int num_dims = static_cast<int>(env->GetArrayLength(shape));
//  jlong* dims = nullptr;
//  if (num_dims > 0) {
//    jboolean is_copy;
//    dims = env->GetLongArrayElements(shape, &is_copy);
//  }
//  static_assert(sizeof(jlong) == sizeof(int64_t),
//                "Java long is not compatible with the TensorFlow C API");
//  // On some platforms "jlong" is a "long" while "int64_t" is a "long long".
//  //
//  // Thus, static_cast<int64_t*>(dims) will trigger a compiler error:
//  // static_cast from 'jlong *' (aka 'long *') to 'int64_t *' (aka 'long long
//  // *') is not allowed
//  //
//  // Since this array is typically very small, use the guaranteed safe scheme of
//  // creating a copy.
//  int64_t* dims_copy = new int64_t[num_dims];
//  for (int i = 0; i < num_dims; ++i) {
//    dims_copy[i] = static_cast<int64_t>(dims[i]);
//  }
//  TF_Tensor* t = TF_AllocateTensor(static_cast<TF_DataType>(dtype), dims_copy,
//                                   num_dims, static_cast<size_t>(sizeInBytes));
//  delete[] dims_copy;
//  if (dims != nullptr) {
//    env->ReleaseLongArrayElements(shape, dims, JNI_ABORT);
//  }
//  if (t == nullptr) {
//    throw_exception(env, kNullPointerException,
//                   "unable to allocate memory for the Tensor");
//    return 0;
//  }
//  return reinterpret_cast<jlong>(t);
//}
//
//JNIEXPORT jlong JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_allocateScalarBytes(
//    JNIEnv* env, jobject object, jbyteArray value) {
//  // TF_STRING tensors are encoded with a table of 8-byte offsets followed by
//  // TF_StringEncode-encoded bytes.
//  size_t src_len = static_cast<int>(env->GetArrayLength(value));
//  size_t dst_len = TF_StringEncodedSize(src_len);
//  TF_Tensor* t = TF_AllocateTensor(TF_STRING, nullptr, 0, 8 + dst_len);
//  char* dst = static_cast<char*>(TF_TensorData(t));
//  memset(dst, 0, 8);  // The offset table
//
//  // jbyte is a signed char, while the C standard doesn't require char and
//  // signed char to be the same. As a result, static_cast<char*>(src) will
//  // complain. Copy the string instead. sigh!
//  jbyte* jsrc = env->GetByteArrayElements(value, nullptr);
//  std::unique_ptr<char[]> src(new char[src_len]);
//  static_assert(sizeof(jbyte) == sizeof(char),
//                "Cannot convert Java byte to a C char");
//  memcpy(src.get(), jsrc, src_len);
//  env->ReleaseByteArrayElements(value, jsrc, JNI_ABORT);
//
//  TF_Status* status = TF_NewStatus();
//  TF_StringEncode(src.get(), src_len, dst + 8, dst_len, status);
//  if (!throw_exception_if_not_ok(env, status)) {
//    TF_DeleteStatus(status);
//    return 0;
//  }
//  TF_DeleteStatus(status);
//  return reinterpret_cast<jlong>(t);
//}
//
//JNIEXPORT void JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_setValue(JNIEnv* env,
//                                                                           jobject object,
//                                                                           jlong handle,
//                                                                           jobject value) {
//  TF_Tensor* t = requireHandle(env, handle);
//  if (t == nullptr) return;
//  int num_dims = TF_NumDims(t);
//  TF_DataType dtype = TF_TensorType(t);
//  void* data = TF_TensorData(t);
//  const size_t sz = TF_TensorByteSize(t);
//  if (num_dims == 0) {
//    writeScalar(env, value, dtype, data, sz);
//  } else {
//    writeNDArray(env, static_cast<jarray>(value), dtype, num_dims,
//                 static_cast<char*>(data), sz);
//  }
//}
//
//#define DEFINE_GET_SCALAR_METHOD(jtype, dtype, method_suffix)                                  \
//  JNIEXPORT jtype JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_scalar##method_suffix(    \
//      JNIEnv* env, jobject object, jlong handle) {                                             \
//    jtype ret = 0;                                                                             \
//    TF_Tensor* t = requireHandle(env, handle);                                                 \
//    if (t == nullptr) return ret;                                                              \
//    if (TF_NumDims(t) != 0) {                                                                  \
//      throwException(env, kIllegalStateException, "Tensor is not a scalar");                   \
//    } else if (TF_TensorType(t) != dtype) {                                                    \
//      throw_exception(env, kIllegalStateException, "Tensor is not a %s scalar",                 \
//                     #method_suffix);                                                          \
//    } else {                                                                                   \
//      memcpy(&ret, TF_TensorData(t), elemByteSize(dtype));                                     \
//    }                                                                                          \
//    return ret;                                                                                \
//  }
//DEFINE_GET_SCALAR_METHOD(jfloat, TF_FLOAT, Float);
//DEFINE_GET_SCALAR_METHOD(jdouble, TF_DOUBLE, Double);
//DEFINE_GET_SCALAR_METHOD(jint, TF_INT32, Int);
//DEFINE_GET_SCALAR_METHOD(jlong, TF_INT64, Long);
//DEFINE_GET_SCALAR_METHOD(jboolean, TF_BOOL, Boolean);
//#undef DEFINE_GET_SCALAR_METHOD
//
//JNIEXPORT jbyteArray JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_scalarBytes(
//    JNIEnv* env, jobject object, jlong handle) {
//  TF_Tensor* t = requireHandle(env, handle);
//  if (t == nullptr) return nullptr;
//  if (TF_NumDims(t) != 0) {
//    throw_exception(env, kIllegalStateException, "Tensor is not a scalar");
//    return nullptr;
//  }
//  if (TF_TensorType(t) != TF_STRING) {
//    throw_exception(env, kIllegalArgumentException,
//                   "Tensor is not a string/bytes scalar");
//    return nullptr;
//  }
//  const char* data = static_cast<const char*>(TF_TensorData(t));
//  const char* src = data + 8;
//  size_t src_len = TF_TensorByteSize(t) - 8;
//  uint64_t offset = 0;
//  memcpy(&offset, data, sizeof(offset));
//  if (offset >= src_len) {
//    throw_exception(env, kIllegalArgumentException,
//                   "invalid tensor encoding: bad offsets");
//    return nullptr;
//  }
//  jbyteArray ret = nullptr;
//  const char* dst = nullptr;
//  size_t dst_len = 0;
//  TF_Status* status = TF_NewStatus();
//  TF_StringDecode(src, src_len, &dst, &dst_len, status);
//  if (throw_exception_if_not_ok(env, status)) {
//    ret = env->NewByteArray(dst_len);
//    jbyte* cpy = env->GetByteArrayElements(ret, nullptr);
//    memcpy(cpy, dst, dst_len);
//    env->ReleaseByteArrayElements(ret, cpy, 0);
//  }
//  TF_DeleteStatus(status);
//  return ret;
//}
//
//JNIEXPORT void JNICALL Java_org_platanios_tensorflow_jni_Tensor_00024_readNDArray(JNIEnv* env,
//                                                                              jobject object,
//                                                                              jlong handle,
//                                                                              jobject value) {
//  TF_Tensor* t = requireHandle(env, handle);
//  if (t == nullptr) return;
//  int num_dims = TF_NumDims(t);
//  TF_DataType dtype = TF_TensorType(t);
//  const void* data = TF_TensorData(t);
//  const size_t sz = TF_TensorByteSize(t);
//  if (num_dims == 0) {
//    throw_exception(env, kIllegalArgumentException,
//                   "copyTo() is not meant for scalar Tensors, use the scalar "
//                   "accessor (floatValue(), intValue() etc.) instead");
//    return;
//  }
//  readNDArray(env, dtype, static_cast<const char*>(data), sz, num_dims,
//              static_cast<jarray>(value));
//}