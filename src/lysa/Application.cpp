/*
* Copyright (c) 2025-present Henri Michelon
*
* This software is released under the MIT License.
* https://opensource.org/licenses/MIT
*/
module lysa.application;

namespace lysa {

    std::shared_ptr<Application> Application::createApplication(ApplicationConfig& applicationConfig) {
        std::cout << "Hello App!" << std::endl;
        return nullptr;
    }

}