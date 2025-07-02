/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.application;

import lysa.input;
import lysa.loader;
import lysa.scene;
import lysa.type_registry;
import lysa.nodes.camera;
import lysa.nodes.collision_area;
import lysa.nodes.directional_light;
import lysa.nodes.environment;
import lysa.nodes.omni_light;
import lysa.nodes.ray_cast;
import lysa.nodes.rigid_body;
import lysa.nodes.spot_light;
import lysa.nodes.static_body;
import lysa.nodes.node;
// using namespace std::chrono;

namespace lysa {

    Application* Application::instance{nullptr};

    Application::Application(ApplicationConfiguration& config) :
        config{config},
        vireo{vireo::Vireo::create(config.backend)},
        graphicQueue{vireo->createSubmitQueue(vireo::CommandType::GRAPHIC, L"Main graphic Queue")},
        computeQueue{vireo->createSubmitQueue(vireo::CommandType::COMPUTE, L"Main compute Queue")},
        resources{*vireo, config.resourcesConfig, *graphicQueue},
        physicsEngine{PhysicsEngine::create(config.physicsConfig)} {
        assert([&]{ return instance == nullptr;}, "Global Application instance already defined");
        instance = this;
        if constexpr (isLoggingEnabled()) {
            log = std::make_shared<Log>();
            Log::open(log);
        }
        Scene::createDescriptorLayouts();
        registerTypes();
    }

    Application::~Application() {
        graphicQueue->waitIdle();
        windows.clear();
        Scene::destroyDescriptorLayouts();
        resources.cleanup();
        Loader::clearCache();
        vireo.reset();
        if constexpr (isLoggingEnabled()) {
            Log::close();
        }
    }

    void Application::run() {
        Input::initInput();
        onReady();
        mainLoop();
        onQuit();
        Input::closeInput();
    }

    void Application::addWindow(const std::shared_ptr<Window>& window) {
        graphicQueue->waitIdle();
        window->mainWindow = windows.empty();
        windows.push_back(window);
    }

    void Application::removeWindow(Window* window) {
        graphicQueue->waitIdle();
        if (window->mainWindow) {
            quit = true;
        }
        windows.remove_if([window](const std::shared_ptr<Window>& w) {
            return w.get() == window;
        });
    }

    void Application::drawFrame() {
        resources.restart();
        if (resources.getSamplers().ipUpdated()) {
            resources.getSamplers().update();
        }

        // Physics events & others deferred calls
        if (!deferredCalls.empty()) {
            std::ranges::for_each(deferredCalls, [](const std::function<void()>& call) {
                call();
            });
            auto lock = std::lock_guard(deferredCallsMutex);
            deferredCalls.clear();
        }

        const double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count();
        double frameTime = newTime - currentTime;

        // https://gafferongames.com/post/fix_your_timestep/
        if (frameTime > 0.25) {
            frameTime = 0.25; // Note: Max frame time to avoid spiral of death
        }
        currentTime = newTime;
        accumulator += frameTime;
        {
            while (accumulator >= FIXED_DELTA_TIME) {
                for (const auto& window : windows) {
                    window->physicsProcess(FIXED_DELTA_TIME);
                }
                accumulator -= FIXED_DELTA_TIME;
            }
            for (const auto& window : windows) {
                window->process(static_cast<float>(accumulator / FIXED_DELTA_TIME));
            }
        }

        for (const auto& window : windows) {
            window->update();
            window->drawFrame();
        }
    }

    void Application::registerTypes() const {
        TypeRegistry::registerType<Camera>("Camera");
        TypeRegistry::registerType<CollisionArea>("CollisionArea");
        TypeRegistry::registerType<DirectionalLight>("DirectionalLight");
        TypeRegistry::registerType<Environment>("Environment");
        TypeRegistry::registerType<Node>("Node");
        TypeRegistry::registerType<OmniLight>("OmniLight");
        TypeRegistry::registerType<RayCast>("RayCast");
        TypeRegistry::registerType<RigidBody>("RigidBody");
        // TypeRegistry::registerType<Skybox>("Skybox");
        TypeRegistry::registerType<SpotLight>("SpotLight");
        TypeRegistry::registerType<StaticBody>("StaticBody");
    }

}