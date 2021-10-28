// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include "lite/api/paddle_use_kernels.h"
#include "lite/api/paddle_use_ops.h"
#include "lite/core/test/arena/framework.h"

namespace paddle {
namespace lite {

class WeightedAddComputeTester : public arena::TestCase {
 protected:
  // common attributes for this op.
  std::string x_ = "x";
  std::string y_ = "y";
  std::string out_ = "out";
  std::string alias_ = "fp32";
  DDim dims_{{10}};

 public:
  WeightedAddComputeTester(const Place& place,
                      const std::string& alias
                      //, DDim x_dims
                      )
      : TestCase(place, alias),
        alias_(alias)
        //, dims_(x_dims) 
        {}

  // template function for RunBaseline according to input_tensor precision type
  // and output tensor precision type(dtype)
  template <typename T>
  void RunBaselineDtype(Scope* scope) {
    auto* x = scope->FindTensor(x_);
    const T* x_data = x->template data<T>();
    auto* y = scope->FindTensor(y_);
    const T* y_data = y->template data<T>();
    auto* out = scope->NewTensor(out_);
    CHECK(out);
    out->Resize(dims_);
    auto* output_data = out->template mutable_data<T>();
    for (int i = 0; i < dims_.production(); i++) {
      output_data[i] = 0.5 * x_data[i] + 2 * y_data[i];
    }
  
  }

  void RunBaseline(Scope* scope) override {
      RunBaselineDtype<float>(scope);
  }

  void PrepareOpDesc(cpp::OpDesc* op_desc) {
    op_desc->SetType("weighted_add");
    op_desc->SetInput("X", {x_});
    op_desc->SetInput("Y", {y_});
    op_desc->SetOutput("Out", {out_});
  }

  void PrepareData() override {
    std::vector<float> dx(dims_.production());
    for (size_t i = 0; i < dx.size(); i++) {
      dx[i] = i * 1.1f;
    }
    SetCommonTensor(x_, dims_, dx.data());

    std::vector<float> dy(dims_.production());
    for (size_t i = 0; i < dy.size(); i++) {
      dy[i] = i * 2.2f;
    }
    SetCommonTensor(y_, dims_, dy.data());
  }
};

void TestWeightedAdd(const Place& place) {
  std::unique_ptr<arena::TestCase> tester(new WeightedAddComputeTester(
      place, "def"));
  arena::Arena arena(std::move(tester), place, 2e-5);
  arena.TestPrecision();
}

TEST(WeightedAdd, precision) {
  Place place;
#if defined(LITE_WITH_X86)
  //place = TARGET(kHost);
  place = TARGET(kX86);
#else
  return;
#endif

  TestWeightedAdd(place);
}

}  // namespace lite
}  // namespace paddle
