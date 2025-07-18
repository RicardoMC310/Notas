#include <iostream>
#include <taskflow/taskflow.hpp>

class EventSystem;

class ISystem
{
public:
    std::shared_ptr<EventSystem> eventSystem;
    std::string name;
    virtual void run() = 0;
};

class CoreSystem
{
    std::unordered_map<std::string, std::shared_ptr<ISystem>> systems;
    std::vector<std::pair<std::string, std::string>> dependencies;

public:
    CoreSystem()
    {
    }
    ~CoreSystem()
    {
    }

    void addSystem(std::string name, std::shared_ptr<ISystem> system)
    {
        systems[name] = system;
    }

    void setDependencies(std::string before, std::string after)
    {
        dependencies.emplace_back(before, after);
    }

    template <typename TSystem>
    std::shared_ptr<TSystem> getSystem(std::string name)
    {
        if (systems.count(name))
        {
            return std::dynamic_pointer_cast<TSystem>(systems[name]);
        }

        return nullptr;
    }

    void run()
    {
        tf::Executor executor{systems.size()};
        tf::Taskflow taskflow;

        std::unordered_map<std::string, tf::Task> tasks;

        for (auto &[name, system] : systems)
        {
            tasks[name] = taskflow.emplace([&system, &name, this]
                                           {system->name = name; system->eventSystem = this->getSystem<EventSystem>("EventSystem");system->run(); });
        }

        for (auto &[before, after] : dependencies)
        {
            if (tasks.count(before) && tasks.count(after))
            {
                tasks[before].precede(tasks[after]);
            }
        }

        executor.run(taskflow).wait();
    }
};

class EventSystem : public ISystem
{
    std::vector<std::pair<std::string, std::function<void(std::string &)>>> events;

public:
    void listener(const std::string &name, std::function<void(std::string &)> func)
    {
        events.emplace_back(name, func);
    }

    void dispatch(const std::string &nameEvent, std::string msg)
    {
        for (auto &[name, func] : events)
        {
            if (nameEvent == name)
            {
                func(msg);
            }
        }
    }

    void run() override
    {
        std::cout << "Rodando o " << this->name << '\n';
    }
};

class RenderSystem : public ISystem
{
public:
    void listenerRender(std::string &msg)
    {
        std::cout << msg << "\n";
    }

    void run() override
    {

        this->eventSystem->listener("Renderizar", [this](std::string &msg)
                                    { listenerRender(msg); });

        auto start = std::chrono::high_resolution_clock::now();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(1, 9372);

        std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));

        std::chrono::duration<double, std::milli> duration = (std::chrono::high_resolution_clock::now() - start);

        std::cout << "[loaded] [name: " << this->name << "] [thread_id: " << std::this_thread::get_id() << "] carregou em " << duration.count() << "ms\n";
    }
};

class WindowSystem : public ISystem
{
public:
    void quit(std::string &msg)
    {
        std::cout << "message de saida: " << msg << '\n';
        exit(0);
    }

    void run() override
    {
        this->eventSystem->listener("quit", [this](std::string &msg)
                                    { quit(msg); });

        auto start = std::chrono::high_resolution_clock::now();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(1, 9372);

        std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));

        std::chrono::duration<double, std::milli> duration = (std::chrono::high_resolution_clock::now() - start);

        std::cout << "[loaded] [name: " << this->name << "] [thread_id: " << std::this_thread::get_id() << "] carregou em " << duration.count() << "ms\n";

        this->eventSystem->dispatch("Renderizar", "renderiza ai irmão");
    }
};

class InputSystem : public ISystem
{
public:
    void run() override
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distrib(1, 9372);

        std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));

        std::chrono::duration<double, std::milli> duration = (std::chrono::high_resolution_clock::now() - start);

        std::cout << "[loaded] [name: " << this->name << "] [thread_id: " << std::this_thread::get_id() << "] carregou em " << duration.count() << "ms\n";

        this->eventSystem->dispatch("Renderizar", "renderiza ai irmão");
        this->eventSystem->dispatch("quit", "saindo com o x");
    }
};

int main()
{
    auto start = std::chrono::high_resolution_clock::now();

    CoreSystem core;

    core.addSystem("EventSystem", std::make_shared<EventSystem>());
    core.addSystem("RenderSystem", std::make_shared<RenderSystem>());
    core.addSystem("InputSystem", std::make_shared<InputSystem>());
    core.addSystem("WindowSystem", std::make_shared<WindowSystem>());

    core.setDependencies("EventSystem", "RenderSystem"); // Event -> render
    core.setDependencies("EventSystem", "InputSystem");  // Event -> input
    core.setDependencies("EventSystem", "WindowSystem"); // Event -> window

    core.run();

    std::chrono::duration<double, std::milli> duration = (std::chrono::high_resolution_clock::now() - start);

    std::cout << "durou " << duration.count() << "ms\n";

    return 0;
}