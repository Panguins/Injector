#pragma once
#include <unordered_map>

#include "NT.h"
#include "x86.h"

using ulong_t = unsigned long;

constexpr bool is86 = sizeof( uintptr_t ) == sizeof( uint32_t );

class c_syscalls {
protected:
	std::unordered_map< std::string, std::pair< uint16_t, uint16_t > > m_syscalls;

	// 16 is very arbitrary... but whatever
	// if something crashes this is why
	__forceinline size_t syscall_wrapper_size( uint8_t* funptr, uint16_t *ret_c_out ) {
		for ( size_t offset{ }; offset < 0x30; offset++ ) {
			if ( funptr[ offset ] == x86::instruction::retn ) {
				if ( ret_c_out )
					*ret_c_out = 0;

				return offset + 1;
			}
			else if ( funptr[ offset ] == x86::instruction::retn_imm16 ) {
				if ( ret_c_out )
					*ret_c_out = *( uint16_t * )( &funptr[ offset + 1 ] );

				return offset + 3;
			}
		}
		return 0;
	}

	__forceinline bool is_syscall( uint8_t* funptr, size_t func_size ) {
		const uint32_t encoded_opcode = x86::encode_mov_imm32( x86::reg::eax );

		if ( /*is86*/ true ? funptr[ 0 ] != encoded_opcode : !( funptr[ 0 ] == 0x4c && funptr[ 1 ] == 0x8b && funptr[ 2 ] == 0xd1 ) )
			return false;

		for ( size_t offset{ }; offset < func_size; offset++ ) {
			if ( true /*is86*/ ) {
				if ( ( funptr[ offset ] == x86::instruction::fs    && // win7
					funptr[ offset + 1 ] == x86::instruction::call ) ||

					( funptr[ offset ] == x86::instruction::call &&  // win10
						funptr[ offset + 1 ] == 0xd2 /*call edx*/ ) )

					return true;

			}

			else {
				if ( funptr[ offset ] == 0x0f && // win7 + win10
					funptr[ offset + 1 ] == 0x05 )
					return true;
			}
		}

		return false;
	}

	__forceinline uint16_t get_syscall_index( uintptr_t func_addr, std::ptrdiff_t *stub_offset = nullptr, uint16_t *ret_c_out = nullptr ) {
		uint8_t* ubp_addr = reinterpret_cast< uint8_t* >( func_addr );
		uint16_t ret_c{ };
		size_t   wrapper_size = syscall_wrapper_size( ubp_addr, &ret_c );

		if ( ret_c_out )
			*ret_c_out = ret_c;

		wrapper_size = ( wrapper_size ) ? wrapper_size : 16;

		if ( is_syscall( ubp_addr, wrapper_size ) ) {
			// mov eax, imm32
			const uint32_t encoded_opcode = x86::encode_mov_imm32( x86::reg::eax );

			for ( size_t offset{ }; offset < wrapper_size; offset++ ) {
				if ( *reinterpret_cast< uint8_t* >( func_addr + offset ) == encoded_opcode ) {
					if ( stub_offset )
						*stub_offset = offset;

					return ( *reinterpret_cast< uint16_t* >( func_addr + offset + 1 ) );
				}
			}
		}

		return 0;
	}

	std::pair< uint8_t*, size_t > m_shellcode_stub;
	void *m_call_table;
public:

	__forceinline ~c_syscalls( ) {
		if ( m_call_table )
			delete[ ] m_call_table;

		if ( m_shellcode_stub.first )
			delete[ ] m_shellcode_stub.first;
	}

	__forceinline c_syscalls( ) :
		m_syscalls{ }, m_shellcode_stub{ } {

		init( );

		// b1gr0fl
		m_call_table = new char[ 0x100000 ];
		memset( m_call_table, 0, 0x100000 );

		if ( true /*x86*/ ) {
			for ( auto& syscall : m_syscalls ) {
				void *stub_addr = ( void* )( uintptr_t( m_call_table ) + ( syscall.second.first * m_shellcode_stub.second ) );
				memcpy( stub_addr, m_shellcode_stub.first, m_shellcode_stub.second );

				std::ptrdiff_t index_offset{ };
				get_syscall_index( ( uintptr_t )stub_addr, &index_offset );

				auto stub_return = ( uint16_t * )( uintptr_t( stub_addr ) + m_shellcode_stub.second - 2 );
				*stub_return = syscall.second.second;

				*( uint32_t * )( uintptr_t( stub_addr ) + index_offset + 1 ) = ( syscall.second.first );

			}
		}
	}

	__forceinline void init( ) {
		uint32_t index;
		uint16_t ret_c{ };

		if ( g_nt.m_exports.empty( ) ) {
			g_nt.dump_exports( );
		}

		for ( const auto& exp : g_nt.m_exports ) {
			index = get_syscall_index( exp.second, nullptr, &ret_c );

			if ( index ) {
				m_syscalls[ exp.first ].first = index;
				m_syscalls[ exp.first ].second = ret_c;

				if ( !m_shellcode_stub.first ) {
					m_shellcode_stub.second = syscall_wrapper_size( reinterpret_cast< uint8_t* >( exp.second ), &ret_c );

					m_shellcode_stub.first = new uint8_t[ m_shellcode_stub.second ];

					m_syscalls[ exp.first ].second = ret_c;

					memcpy( m_shellcode_stub.first, reinterpret_cast< void* >( exp.second ), m_shellcode_stub.second );
				}
			}
		}
	}

	template< typename t = void* >
	__forceinline t get_syscall_func( std::string name ) {
		return ( t )( ( uintptr_t( m_call_table ) + ( get_syscall( name ) * m_shellcode_stub.second ) ) );
	}

	__forceinline uint16_t get_syscall( std::string name ) {
		return m_syscalls[ name ].first;
	}

	__forceinline auto& get_syscalls( ) {
		return m_syscalls;
	}
};

extern c_syscalls g_syscalls;