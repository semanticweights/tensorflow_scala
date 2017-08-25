/* DO NOT EDIT THIS FILE - it is machine generated */

/* Copyright 2017, Emmanouil Antonios Platanios. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include "tensor_basic_ops.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <sstream>

#include "include/c_api.h"
#include "include/c_eager_api.h"
#include "include/exception_jni.h"
#include "include/utilities.h"

JNIEXPORT jlong JNICALL Java_org_platanios_tensorflow_jni_generated_tensors_Basic_00024_pack(
    JNIEnv* env, jobject object, jlong context_handle, jlongArray values, jlong axis) {
  REQUIRE_HANDLE(context, TFE_Context, context_handle, 0);
  std::unique_ptr<TF_Status, decltype(&TF_DeleteStatus)> status(TF_NewStatus(), TF_DeleteStatus);

  std::unique_ptr<TFE_Op, decltype(&TFE_DeleteOp)> op(
      TFE_NewOp(context, "Pack", status.get()), TFE_DeleteOp);
  CHECK_STATUS(env, status.get(), 0);

  const int values_num_tensors = env->GetArrayLength(values);
  jlong *values_elems = env->GetLongArrayElements(values, nullptr);
  for (int i = 0; i < values_num_tensors; ++i) {
    REQUIRE_HANDLE(tensor_handle, TFE_TensorHandle, values_elems[i], 0);
    TFE_OpAddInput(op.get(), tensor_handle, status.get());
    CHECK_STATUS(env, status.get(), 0);
  }
  env->ReleaseLongArrayElements(values, values_elems, JNI_ABORT);

  const int values_attr_N = env->GetArrayLength(values);

  const int values_attr_T_num_tensors = env->GetArrayLength(values);
  jlong *values_attr_T_elems = env->GetLongArrayElements(values, nullptr);

  REQUIRE_HANDLE(values_attr_T_elems_head, TFE_TensorHandle, values_attr_T_elems[0], 0);
  const TF_DataType values_attr_T = TFE_TensorHandleDataType(values_attr_T_elems_head);

  for (int i = 0; i < values_attr_T_num_tensors; ++i) {
    REQUIRE_HANDLE(tensor, TFE_TensorHandle, values_attr_T_elems[i], 0);
    const TF_DataType data_type = TFE_TensorHandleDataType(tensor);
    if (values_attr_T != data_type) {
      std::stringstream error_msg;
      error_msg
          << "Argument 'values' of 'pack' op with data type '"
          << data_type
          << "' must match data type '"
          << values_attr_T
          << "' of argument 'values'";
      throw_exception(env, jvm_illegal_argument_exception, error_msg.str().c_str());
    }
  }
  env->ReleaseLongArrayElements(values, values_attr_T_elems, JNI_ABORT);

  TFE_OpSetAttrInt(op.get(), "axis", static_cast<int64_t>(axis));

  const int num_outputs = 1;
  std::unique_ptr<TFE_TensorHandle* []> outputs(new TFE_TensorHandle* [num_outputs]);
  std::unique_ptr<int[]> actual_num_outputs(new int[1] {1});
  TFE_Execute(op.get(), outputs.get(), actual_num_outputs.get(), status.get());
  CHECK_STATUS(env, status.get(), 0);

  return reinterpret_cast<jlong>(outputs[0]);
}

JNIEXPORT jlong JNICALL Java_org_platanios_tensorflow_jni_generated_tensors_Basic_00024_stridedSlice(
    JNIEnv* env, jobject object, jlong context_handle, jlong input, jlong begin, jlong end, jlong strides, jlong begin_mask, jlong end_mask, jlong ellipsis_mask, jlong new_axis_mask, jlong shrink_axis_mask) {
  REQUIRE_HANDLE(context, TFE_Context, context_handle, 0);
  std::unique_ptr<TF_Status, decltype(&TF_DeleteStatus)> status(TF_NewStatus(), TF_DeleteStatus);

  std::unique_ptr<TFE_Op, decltype(&TFE_DeleteOp)> op(
      TFE_NewOp(context, "StridedSlice", status.get()), TFE_DeleteOp);
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(input_tensor_handle, TFE_TensorHandle, input, 0);
  TFE_OpAddInput(op.get(), input_tensor_handle, status.get());
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(begin_tensor_handle, TFE_TensorHandle, begin, 0);
  TFE_OpAddInput(op.get(), begin_tensor_handle, status.get());
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(end_tensor_handle, TFE_TensorHandle, end, 0);
  TFE_OpAddInput(op.get(), end_tensor_handle, status.get());
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(strides_tensor_handle, TFE_TensorHandle, strides, 0);
  TFE_OpAddInput(op.get(), strides_tensor_handle, status.get());
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(input_attr_T_input_tensor_h, TFE_TensorHandle, input, 0);
  const TF_DataType input_attr_T = TFE_TensorHandleDataType(input_attr_T_input_tensor_h);

  REQUIRE_HANDLE(begin_attr_Index_begin_tensor_h, TFE_TensorHandle, begin, 0);
  const TF_DataType begin_attr_Index = TFE_TensorHandleDataType(begin_attr_Index_begin_tensor_h);

  REQUIRE_HANDLE(begin_attr_Index_end_tensor_h, TFE_TensorHandle, end, 0);
  const TF_DataType end_attr_Index = TFE_TensorHandleDataType(begin_attr_Index_end_tensor_h);
  if (begin_attr_Index != end_attr_Index) {
      std::stringstream error_msg;
      error_msg
          << "Argument 'end' of 'stridedSlice' op with data type '"
          << end_attr_Index
          << "' must match data type '"
          << begin_attr_Index
          << "' of argument 'begin'";
      throw_exception(env, jvm_illegal_argument_exception, error_msg.str().c_str());
  }

  REQUIRE_HANDLE(begin_attr_Index_strides_tensor_h, TFE_TensorHandle, strides, 0);
  const TF_DataType strides_attr_Index = TFE_TensorHandleDataType(begin_attr_Index_strides_tensor_h);
  if (begin_attr_Index != strides_attr_Index) {
      std::stringstream error_msg;
      error_msg
          << "Argument 'strides' of 'stridedSlice' op with data type '"
          << strides_attr_Index
          << "' must match data type '"
          << begin_attr_Index
          << "' of argument 'begin'";
      throw_exception(env, jvm_illegal_argument_exception, error_msg.str().c_str());
  }

  TFE_OpSetAttrInt(op.get(), "begin_mask", static_cast<int64_t>(begin_mask));

  TFE_OpSetAttrInt(op.get(), "end_mask", static_cast<int64_t>(end_mask));

  TFE_OpSetAttrInt(op.get(), "ellipsis_mask", static_cast<int64_t>(ellipsis_mask));

  TFE_OpSetAttrInt(op.get(), "new_axis_mask", static_cast<int64_t>(new_axis_mask));

  TFE_OpSetAttrInt(op.get(), "shrink_axis_mask", static_cast<int64_t>(shrink_axis_mask));

  const int num_outputs = 1;
  std::unique_ptr<TFE_TensorHandle* []> outputs(new TFE_TensorHandle* [num_outputs]);
  std::unique_ptr<int[]> actual_num_outputs(new int[1] {1});
  TFE_Execute(op.get(), outputs.get(), actual_num_outputs.get(), status.get());
  CHECK_STATUS(env, status.get(), 0);

  return reinterpret_cast<jlong>(outputs[0]);
}

JNIEXPORT jlong JNICALL Java_org_platanios_tensorflow_jni_generated_tensors_Basic_00024_reshape(
    JNIEnv* env, jobject object, jlong context_handle, jlong tensor, jlong shape) {
  REQUIRE_HANDLE(context, TFE_Context, context_handle, 0);
  std::unique_ptr<TF_Status, decltype(&TF_DeleteStatus)> status(TF_NewStatus(), TF_DeleteStatus);

  std::unique_ptr<TFE_Op, decltype(&TFE_DeleteOp)> op(
      TFE_NewOp(context, "Reshape", status.get()), TFE_DeleteOp);
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(tensor_tensor_handle, TFE_TensorHandle, tensor, 0);
  TFE_OpAddInput(op.get(), tensor_tensor_handle, status.get());
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(shape_tensor_handle, TFE_TensorHandle, shape, 0);
  TFE_OpAddInput(op.get(), shape_tensor_handle, status.get());
  CHECK_STATUS(env, status.get(), 0);

  REQUIRE_HANDLE(tensor_attr_T_tensor_tensor_h, TFE_TensorHandle, tensor, 0);
  const TF_DataType tensor_attr_T = TFE_TensorHandleDataType(tensor_attr_T_tensor_tensor_h);

  REQUIRE_HANDLE(shape_attr_Tshape_shape_tensor_h, TFE_TensorHandle, shape, 0);
  const TF_DataType shape_attr_Tshape = TFE_TensorHandleDataType(shape_attr_Tshape_shape_tensor_h);

  const int num_outputs = 1;
  std::unique_ptr<TFE_TensorHandle* []> outputs(new TFE_TensorHandle* [num_outputs]);
  std::unique_ptr<int[]> actual_num_outputs(new int[1] {1});
  TFE_Execute(op.get(), outputs.get(), actual_num_outputs.get(), status.get());
  CHECK_STATUS(env, status.get(), 0);

  return reinterpret_cast<jlong>(outputs[0]);
}