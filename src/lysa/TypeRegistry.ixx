/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
export module lysa.type_registry;

import std;
import lysa.object;
import lysa.global;

export namespace lysa {

    // template<typename T> Object* _createNewObjectInstance() { return new T(); }

    /**
     * Register custom nodes types used to map the node's description in JSON scene files and real classes
     */
    class TypeRegistry {
    public:
        /**
         * Creates a new shared pointer to a new instance of type `clazz` with casting to the type `T`
         */
        template<typename T> static std::shared_ptr<T> makeShared(const std::string&clazz) {
            if (!typeMap->contains(clazz)) { throw Exception("Type", clazz, "not registered in TypeRegistry"); }
            return std::shared_ptr<T>(reinterpret_pointer_cast<T>(typeMap->at(clazz)()));
        }

        /**
         * Register a new class. Use it when registering from code.
         * If you want to register outside a bloc of code use the `Z0_REGISTER_TYPE(class)` macro after your class declaration
         */
        template<typename T> static void registerType(const std::string&clazz) {
            if (typeMap == nullptr) { typeMap = std::make_unique<std::map<std::string, std::function<std::shared_ptr<Object>()>>>(); }
            typeMap->emplace(clazz, []{ return std::make_shared<T>(); });
        }

    // private:
        static std::unique_ptr<std::map<std::string, std::function<std::shared_ptr<Object>()>>> typeMap;
    };

    template<typename T>
    struct _TypeRegister {
        explicit _TypeRegister(const std::string&clazz) {
            TypeRegistry::registerType<T>(clazz);
        }
    };

    std::unique_ptr<std::map<std::string, std::function<std::shared_ptr<Object>()>>> TypeRegistry::typeMap{nullptr};

}