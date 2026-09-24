#pragma once
#include <chrono>
#include <ctime>
#include <vector>
#include <Windows.h>
#include <fstream>
#include <winternl.h>
#include <cstdint>
#include <dia2.h>
#include <diacreate.h>
#include <atlbase.h>
#include <string>
#include <memory>

#pragma comment(lib, "diaguids.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "ntdll.lib")

#include <dependencies/ia32/include.h>
#include <dependencies/oxorany/include.h>

#include <workspace/utility/logger.hxx>
#include <workspace/utility/utility.hxx>

#include <impl/pdb/pdb.hxx>
auto g_pdb = std::make_shared<pdb::c_pdb>( oxorany( "ntoskrnl.exe" ) );

#include <workspace/core/service/bytes.h>
#include <workspace/core/service/service.hxx>

#include <workspace/core/service/driver/driver.hxx>
auto g_driver = std::make_shared<driver::c_driver>( );

#include <workspace/core/module/module.hxx>
#include <workspace/core/module/exports/exports.h>

#include <workspace/core/memory/physical/physical.hxx>
#include <workspace/core/memory/paging/paging.hxx>
auto g_paging = std::make_shared<paging::c_paging>( );

#include <workspace/core/memory/syscall/syscall.hxx>
auto g_syscall = std::make_shared<syscall::c_syscall>( );

#include <workspace/core/service/cleanup/orchestrate.hxx>

#include <workspace/core/service/startup.hxx>
auto g_service = std::make_shared<service::c_service>( );

#include <workspace/core/map/pe.h>
#include <workspace/core/map/resolve/buddy_resolve.hxx>
#include <workspace/core/map/allocator/allocator.hxx>
#include <workspace/core/map/dependency/dependency.hxx>
#include <workspace/core/map/map.hxx>

#include <workspace/core/module/exports/exports.hxx>
#include <workspace/utility/exit.hxx>
