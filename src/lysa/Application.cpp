/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.application;

import lysa.scene;

namespace lysa {

    Application* Application::instance{nullptr};

    Application::Application(ApplicationConfiguration& config):
        config{config},
        vireo{vireo::Vireo::create(config.backend)},
        resources{*vireo, config.resourcesConfig} {
        assert([&]{ return instance == nullptr;}, "Global Application instance already defined");
        instance = this;
    }

    Application::~Application() {
        Scene::descriptorLayout.reset();
        resources.cleanup();
        vireo.reset();
    }

}