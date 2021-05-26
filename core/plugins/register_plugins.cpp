#include "NvInferPlugin.h"
#include "NvInferPluginUtils.h"
#include "core/plugins/impl/interpolate_plugin.h"
#include "core/plugins/impl/normalize_plugin.h"
#include "core/plugins/plugins.h"
#include "core/util/prelude.h"

namespace trtorch {
namespace core {
namespace plugins {
namespace impl {

// Main registry for all flavours of plugins (eg: TRT plugins, TRTorch opensourced plugins)
class TRTorchPluginRegistry {
 public:
  TRTorchPluginRegistry() {
    // register libNvInferPlugins and TRTorch plugins
    // trtorch_logger logging level is set to kERROR and reset back to kDEBUG.
    // This is because initLibNvInferPlugins initializes only a subset of plugins and logs them.
    // Plugins outside this subset in TensorRT are not being logged in this. So temporarily we disable this to prevent
    // multiple logging of same plugins. To provide a clear list of all plugins, we iterate through getPluginRegistry()
    // where it prints the list of all the plugins registered in TensorRT with their namespaces.
    plugin_logger.set_reportable_log_level(util::logging::LogLevel::kERROR);
    initLibNvInferPlugins(&plugin_logger, "");
    plugin_logger.set_reportable_log_level(util::logging::get_logger().get_reportable_log_level());

    int numCreators = 0;
    auto pluginsList = getPluginRegistry()->getPluginCreatorList(&numCreators);
    for (int k = 0; k < numCreators; ++k) {
      if (!pluginsList[k]) {
        plugin_logger.log(util::logging::LogLevel::kDEBUG, "Plugin creator for plugin " + str(k) + " is a nullptr");
        continue;
      }
      std::string pluginNamespace = pluginsList[k]->getPluginNamespace();
      plugin_logger.log(
          util::logging::LogLevel::kDEBUG,
          "Registered plugin creator - " + std::string(pluginsList[k]->getPluginName()) +
              ", Namespace: " + pluginNamespace);
    }
    plugin_logger.log(util::logging::LogLevel::kDEBUG, "Total number of plugins registered: " + str(numCreators));
  }

 public:
  util::logging::TRTorchLogger& plugin_logger = util::logging::get_logger();
};

namespace {
static TRTorchPluginRegistry plugin_registry;
}

} // namespace impl
} // namespace plugins
} // namespace core
} // namespace trtorch
