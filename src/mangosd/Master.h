/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \addtogroup mangosd
/// @{
/// \file

#ifndef _MASTER_H
#define _MASTER_H

#define BOOST_BIND_NO_PLACEHOLDERS 1

#include "Common.h"
#include "Policies/Singleton.h"
#include "Network/AsyncListener.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <thread>
#include <vector>

class WorldSocket;

/// Start the server
class Master
{
    public:
        int Run();
        static volatile bool m_canBeKilled;

#ifdef POCKET_EMBEDDED
        // ---- Pocket Realm embeddable lifecycle hooks ----
        // Run() is a blocking monolith (PID file, DB, world init, signals, CLI
        // thread, network block, teardown) designed for a standalone process.
        // The embeddable runtime (native/pocket-runtime) needs the same pieces
        // but without signal handlers, the console thread, the blocking wait,
        // or process exit. These hooks expose just the reusable phases so the
        // facade can drive them individually on its own worker thread.
        //
        // See docs/patches/native-source-patches.md for why each exists.

        // Open all four databases + version checks + realmID + clearOnline.
        // Returns false (logged) on any failure; never calls exit().
        bool StartDatabasesEmbedded();

        // Attempt World::SetInitialWorldSettings. Under POCKET_EMBEDDED the
        // client-data gates (.map/.dbc) throw fatal_error instead of exit()ing;
        // returns false and sets client_data_gate when caught.
        bool InitWorldEmbedded(bool* client_data_gate);

        // Start the world thread + network listeners (loopback). No signals,
        // no CLI thread. The io_contexts are run on internal threads.
        bool StartNetworkEmbedded(uint32_t network_threads);

        // Cooperative stop + full teardown (the tail of Run()). Resets the
        // process-global state so a second cycle can re-init.
        void StopEmbedded();
#endif

    private:
        bool _StartDB();

        void _HookSignals();
        void _UnhookSignals();
        static void _OnSignal(int s);

        void clearOnlineAccounts();

        boost::asio::io_context m_context;
        boost::asio::io_context m_raContext;

#ifdef POCKET_EMBEDDED
        // Embedded-mode owned resources (the standalone Run() uses locals).
        std::unique_ptr<MaNGOS::Thread> m_worldThread;
        std::unique_ptr<MaNGOS::AsyncListener<WorldSocket>> m_worldListener;
        std::vector<std::thread> m_netThreads;
#endif
    };

#define sMaster MaNGOS::Singleton<Master>::Instance()
#endif
/// @}
