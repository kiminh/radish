/*
 * File: model_io.cc
 * Project: train
 * Author: koth (Koth Chen)
 * -----
 * Last Modified: 2019-09-24 4:09:12
 * Modified By: koth (nobody@verycool.com)
 * -----
 * Copyright 2020 - 2019
 */
#include "radish/train/model_io.h"

#include "radish/utils/logging.h"
#include "radish/utils/tensor_util.h"
namespace radish {
namespace train {

void SaveModel(std::shared_ptr<torch::nn::Module> module,
               const std::string& file_name) {
  torch::serialize::OutputArchive archive;
  auto params = module->named_parameters(true /*recurse*/);
  auto buffers = module->named_buffers(true /*recurse*/);
  for (const auto& val : params) {
    if (!radish::utils::IsEmpty(val.value())) {
      archive.write(val.key(), val.value());
    }
  }
  for (const auto& val : buffers) {
    if (!radish::utils::IsEmpty(val.value())) {
      archive.write(val.key(), val.value(), /*is_buffer*/ true);
    }
  }
  archive.save_to(file_name);
}

void LoadModel(std::shared_ptr<torch::nn::Module> module,
               const std::string& path, const std::string& ignore_name_regex,
               torch::Device device) {
  torch::serialize::InputArchive archive;
  archive.load_from(path, device);
  torch::NoGradGuard no_grad;
  std::regex re(ignore_name_regex);
  std::smatch m;
  auto params = module->named_parameters(true /*recurse*/);
  auto buffers = module->named_buffers(true /*recurse*/);
  for (auto& val : params) {
    if (!std::regex_match(val.key(), m, re)) {
      archive.read(val.key(), val.value());
    }
  }
  for (auto& val : buffers) {
    if (!std::regex_match(val.key(), m, re)) {
      archive.read(val.key(), val.value(), /*is_buffer*/ true);
    }
  }
}

void LoadModelEx(std::shared_ptr<torch::nn::Module> module,
                 const std::string& path, const std::string& prefixVarName,
                 torch::Device device) {
  torch::serialize::InputArchive archive;
  archive.load_from(path, device);
  torch::NoGradGuard no_grad;
  auto params = module->named_parameters(true /*recurse*/);
  auto buffers = module->named_buffers(true /*recurse*/);
  for (auto& val : params) {
    std::string kn = val.key();
    if (prefixVarName.empty() || (kn.size() >= prefixVarName.size() &&
                                  strncmp(kn.c_str(), prefixVarName.c_str(),
                                          prefixVarName.size()) == 0)) {
      archive.read(kn, val.value());
      spdlog::info("load pretrained weights:{}", kn);
    }
  }
  for (auto& val : buffers) {
    std::string kn = val.key();
    if (prefixVarName.empty() || (kn.size() >= prefixVarName.size() &&
                                  strncmp(kn.c_str(), prefixVarName.c_str(),
                                          prefixVarName.size()) == 0)) {
      archive.read(val.key(), val.value(), /*is_buffer*/ true);
      spdlog::info("load pretrained buffer:{}", kn);
    }
  }
}

}  // namespace train
}  // namespace radish