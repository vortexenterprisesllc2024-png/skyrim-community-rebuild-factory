#pragma once

namespace RE::BSScript
{
    class IVirtualMachine;
}

namespace AdventureXP::Papyrus
{
    bool Register(RE::BSScript::IVirtualMachine* vm);
}
