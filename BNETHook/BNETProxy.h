#pragma once

class Module;
Module *ProxyCreateModule();

typedef Module* (*CREATEMODULE)();
extern CREATEMODULE realCreateModule;