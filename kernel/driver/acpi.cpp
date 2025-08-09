#include <acpi.h>

void ACPIManager::Init(XSDP *xsdp)
{
    root = xsdp->XsdtAddress;
    table = (ACPISDTHeader**)(xsdp->XsdtAddress + 1);
}

ACPISDTHeader* ACPIManager::FindTable(const char* name)
{
    for (u32 i = 0; i < root->Length; ++i)
    {
        if (!KernelUtil::Strncmp(name, table[i]->Signature, 4))
            return table[i];
    }
    return nullptr;
}
