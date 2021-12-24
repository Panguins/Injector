#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <winternl.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
//#include <ntdef.h>

using export_t = std::pair< std::string, uintptr_t >;
using ulong_t = unsigned long;

namespace {
	template < typename t > __forceinline t get_ntdll_func( std::string funcname ) {
		return reinterpret_cast< t >( GetProcAddress( GetModuleHandleA( "ntdll.dll" ), funcname.c_str( ) ) );
	}
}

namespace nt
{
	__forceinline void RtlInitUnicodeString( PUNICODE_STRING DestinationString, PWSTR SourceString ) {
		DestinationString->Buffer = SourceString;
		DestinationString->MaximumLength = DestinationString->Length = USHORT( wcslen( SourceString ) * sizeof( WCHAR ) );
	}

	__forceinline void RtlFreeUnicodeString( PUNICODE_STRING DestinationString ) {
		DestinationString->Buffer = nullptr;
		DestinationString->MaximumLength = 0;
	}

	typedef struct _CLIENT_ID {
		HANDLE         UniqueProcess;
		HANDLE         UniqueThread;
	} CLIENT_ID;

	enum HARDERROR_RESPONSE {
		ResponseReturnToCaller,
		ResponseNotHandled,
		ResponseAbort,
		ResponseCancel,
		ResponseIgnore,
		ResponseNo,
		ResponseOk,
		ResponseRetry,
		ResponseYes
	};

	enum HARDERROR_RESPONSE_OPTION {
		OptionAbortRetryIgnore,
		OptionOk,
		OptionOkCancel,
		OptionRetryCancel,
		OptionYesNo,
		OptionYesNoCancel,
		OptionShutdownSystem
	};

	enum MEMORY_INFORMATION_CLASS {
		MemoryBasicInformation,
		MemoryWorkingSetList,
		MemorySectionName,
		MemoryBasicVlmInformation
	};

	using NTReadVirtualMemory		= NTSTATUS( __stdcall* )( HANDLE, const void*, void*, ulong_t, size_t* );
	using NTWriteVirtualMemory		= NTSTATUS( __stdcall* )( HANDLE, void*, void*, ulong_t, size_t* );
	using NTAllocateVirtualMemory	= NTSTATUS( __stdcall* )( HANDLE, void**, unsigned long, size_t*, ulong_t, ulong_t );
	using NTFreeVirtualMemory		= NTSTATUS( __stdcall* )( HANDLE, void**, size_t*, ulong_t );
	using NTProtectVirtualMemory	= NTSTATUS( __stdcall* )( HANDLE, void**, size_t*, ulong_t, ulong_t* );
	using NTQueryVirtualMemory		= NTSTATUS( __stdcall* )( HANDLE, void*, MEMORY_INFORMATION_CLASS, void*, size_t, size_t* );
	using NTOpenProcess				= NTSTATUS( __stdcall* )( HANDLE*, ACCESS_MASK, const OBJECT_ATTRIBUTES*, const CLIENT_ID* );
	using NTClose					= NTSTATUS( __stdcall* )( HANDLE );
	using NTRaiseHardError			= NTSTATUS( __stdcall* )( NTSTATUS, ulong_t, void**, int, void* );

	typedef NTSTATUS( __stdcall* NtCreateFile_t )(
		PHANDLE handle,
		ACCESS_MASK access,
		POBJECT_ATTRIBUTES attr,
		PIO_STATUS_BLOCK io,
		PLARGE_INTEGER alloc_size,
		ULONG attributes,
		ULONG sharing,
		ULONG disposition,
		ULONG options,
		PVOID ea_buffer,
		ULONG ea_length
		);

	typedef NTSTATUS( __stdcall* NtFsControlFile_t )(
		HANDLE           handle,
		HANDLE           event,
		PIO_APC_ROUTINE  apc,
		PVOID            apc_context,
		PIO_STATUS_BLOCK io,
		ULONG            code,
		PVOID            in_buffer,
		ULONG            in_size,
		PVOID            out_buffer,
		ULONG            out_size
		);

	typedef NTSTATUS( __stdcall* NtWaitForSingleObject_t )(
		IN HANDLE Handle,
		IN BOOLEAN Alertable,
		IN PLARGE_INTEGER Timeout OPTIONAL
		);

	typedef NTSTATUS( __stdcall* NtDeviceIoControlFile_t )(
		IN HANDLE FileHandle,
		IN HANDLE Event OPTIONAL,
		IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN PVOID ApcContext OPTIONAL,
		OUT PIO_STATUS_BLOCK IoStatusBlock,
		IN ULONG IoControlCode,
		IN PVOID InputBuffer OPTIONAL,
		IN ULONG InputBufferLength,
		OUT PVOID OutputBuffer OPTIONAL,
		IN ULONG OutputBufferLength
		);

	using NTCreateThreadEx = NTSTATUS( __stdcall* )( 
			void**, ACCESS_MASK, const OBJECT_ATTRIBUTES*,
			HANDLE, LPTHREAD_START_ROUTINE, void*, int, 
			ulong_t, ulong_t, ulong_t, void* ); //winapi has like 200 params in each function rofl

	using NTOpenThread = NTSTATUS( __stdcall* )( void**, ACCESS_MASK, POBJECT_ATTRIBUTES, CLIENT_ID* );
	using NTSuspendThread = NTSTATUS( __stdcall* )( void*, void* );
	using NTResumeThread = NTSTATUS( __stdcall* )( void*, void* );
	using NTGetContextThread = NTSTATUS( __stdcall* )( void*, CONTEXT* );
	using NTSetContextThread = NTSTATUS( __stdcall* )( void*, CONTEXT* );

	static constexpr NTSTATUS STATUS_SUCCESS = 0x0;
	static constexpr NTSTATUS STATUS_SEVERITY_INFORMATIONAL = 0x1;
	static constexpr NTSTATUS STATUS_SEVERITY_WARNING = 0x2;
	static constexpr NTSTATUS STATUS_SEVERITY_ERROR = 0x4;

	__forceinline auto get_ntdll( ) {
		auto teb = reinterpret_cast< PTEB >( __readfsdword( reinterpret_cast< uintptr_t >( &static_cast< NT_TIB* >( nullptr )->Self ) ) );
		auto peb = teb->ProcessEnvironmentBlock;
		auto data_table_entry = reinterpret_cast< PLDR_DATA_TABLE_ENTRY >( peb->Ldr->InMemoryOrderModuleList.Flink[ 0 ].Flink )->Reserved2[ 0 ];
		uintptr_t nt_ = reinterpret_cast< uintptr_t >( data_table_entry );
		return nt_;
	}

	class c_nt {
	public:
		c_nt( ) {
			m_base = get_ntdll( );
		};
		c_nt( uintptr_t nt_handle ) {
			m_base = nt_handle;
		}

		void dump_exports( ) {
			if ( !m_base ) m_base = get_ntdll( );
			auto nt_headers = reinterpret_cast< IMAGE_NT_HEADERS* >( m_base + reinterpret_cast< IMAGE_DOS_HEADER* >( m_base )->e_lfanew );
			auto export_dir = reinterpret_cast< IMAGE_EXPORT_DIRECTORY* >( m_base + nt_headers->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress );

			uintptr_t* names = reinterpret_cast< uintptr_t* >( m_base + export_dir->AddressOfNames );
			uintptr_t* funcs = reinterpret_cast< uintptr_t* >( m_base + export_dir->AddressOfFunctions );
			auto ords = reinterpret_cast< uint16_t* >( m_base + export_dir->AddressOfNameOrdinals );
			
			if ( names && funcs && ords ) {
				for ( uint32_t i{ }; i < export_dir->NumberOfNames; ++i ) {
					m_exports.push_back( { reinterpret_cast< const char* >( m_base + names[ i ] ), m_base + funcs[ ords[ i ] ] } );
				}
			}
			
		}

		void dump_to_file( ) {
			std::ofstream s( "dump.txt" );
			for ( auto& it : m_exports ) {
				s << it.first << ":" << std::hex << it.second << '\n';
			}
		}

		std::vector< export_t > m_exports{ };
		uintptr_t m_base{ };
	};
}

extern nt::c_nt g_nt;