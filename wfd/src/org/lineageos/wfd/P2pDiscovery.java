/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lineageos.wfd;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ChannelListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.net.wifi.p2p.WifiP2pWfdInfo;
import android.os.Looper;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

final class P2pDiscovery {
    private static final String TAG = "WfdProvider";

    private static final int WFD_DEFAULT_CONTROL_PORT = 7236;

    private static final int WFD_MAX_THROUGHPUT_MBPS = 50;

    interface Listener {

        void onSinksChanged(List<WifiP2pDevice> sinks);

        void onP2pStateChanged(boolean enabled);
    }

    private final Context mContext;
    private final Listener mListener;
    private final WifiP2pManager mManager;

    private Channel mChannel;
    private BroadcastReceiver mReceiver;
    private boolean mStarted;

    P2pDiscovery(Context context, Listener listener) {
        mContext = context;
        mListener = listener;
        mManager = (WifiP2pManager) context.getSystemService(Context.WIFI_P2P_SERVICE);
    }

    boolean isSupported() {
        return mManager != null;
    }

    WifiP2pManager getManager() {
        return mManager;
    }

    Channel getChannel() {
        return mChannel;
    }

    static boolean isWfdSink(WifiP2pDevice d) {
        final WifiP2pWfdInfo w = d.wfdInfo;
        if (w == null || !w.isWfdEnabled()) {
            return false;
        }
        final int type = w.getDeviceType();
        return type == WifiP2pWfdInfo.PRIMARY_SINK
                || type == WifiP2pWfdInfo.SECONDARY_SINK
                || type == WifiP2pWfdInfo.SOURCE_OR_PRIMARY_SINK;
    }

    void start() {
        if (mStarted) {
            return;
        }
        if (mManager == null) {
            Log.w(TAG, "P2pDiscovery: WifiP2pManager unavailable");
            return;
        }
        mChannel = mManager.initialize(mContext, Looper.getMainLooper(), new ChannelListener() {
            @Override
            public void onChannelDisconnected() {
                Log.w(TAG, "P2pDiscovery: P2P channel disconnected");
                mChannel = null;
            }
        });
        if (mChannel == null) {
            Log.e(TAG, "P2pDiscovery: failed to initialize P2P channel");
            return;
        }
        mStarted = true;

        final IntentFilter filter = new IntentFilter();
        filter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        filter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        mReceiver = new P2pReceiver();
        mContext.registerReceiver(mReceiver, filter);

        advertiseAsWfdSource();
        discover();
    }

    void stop() {
        if (!mStarted) {
            return;
        }
        mStarted = false;
        if (mReceiver != null) {
            try {
                mContext.unregisterReceiver(mReceiver);
            } catch (IllegalArgumentException ignored) {
            }
            mReceiver = null;
        }
        if (mManager != null && mChannel != null) {
            mManager.stopPeerDiscovery(mChannel, null);

            final WifiP2pWfdInfo off = new WifiP2pWfdInfo();
            off.setWfdEnabled(false);
            mManager.setWFDInfo(mChannel, off, null);
        }
    }

    private void advertiseAsWfdSource() {
        final WifiP2pWfdInfo info = new WifiP2pWfdInfo();
        info.setWfdEnabled(true);
        info.setDeviceType(WifiP2pWfdInfo.WFD_SOURCE);
        info.setSessionAvailable(true);
        info.setControlPort(WFD_DEFAULT_CONTROL_PORT);
        info.setMaxThroughput(WFD_MAX_THROUGHPUT_MBPS);
        mManager.setWFDInfo(mChannel, info, new ActionListener() {
            @Override
            public void onSuccess() {
                Log.i(TAG, "P2pDiscovery: advertising as WFD source (WFD IE on)");
            }

            @Override
            public void onFailure(int reason) {
                Log.e(TAG, "P2pDiscovery: setWFDInfo failed, reason=" + reason);
            }
        });
    }

    private void discover() {
        mManager.discoverPeers(mChannel, new ActionListener() {
            @Override
            public void onSuccess() {
                Log.i(TAG, "P2pDiscovery: discoverPeers started");
            }

            @Override
            public void onFailure(int reason) {

                Log.e(TAG, "P2pDiscovery: discoverPeers failed, reason=" + reason);
            }
        });
    }

    private void requestPeers() {
        if (mManager == null || mChannel == null) {
            return;
        }
        mManager.requestPeers(mChannel, new PeerListListener() {
            @Override
            public void onPeersAvailable(WifiP2pDeviceList peers) {
                final List<WifiP2pDevice> sinks = new ArrayList<>();
                for (WifiP2pDevice d : peers.getDeviceList()) {
                    if (isWfdSink(d)) {
                        sinks.add(d);
                    }
                }
                Log.d(TAG, "P2pDiscovery: " + peers.getDeviceList().size()
                        + " peers, " + sinks.size() + " WFD sink(s)");
                mListener.onSinksChanged(sinks);
            }
        });
    }

    private final class P2pReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action)) {
                final int state = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE,
                        WifiP2pManager.WIFI_P2P_STATE_DISABLED);
                final boolean enabled = state == WifiP2pManager.WIFI_P2P_STATE_ENABLED;
                Log.d(TAG, "P2pDiscovery: P2P state enabled=" + enabled);
                mListener.onP2pStateChanged(enabled);
            } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
                requestPeers();
            }
        }
    }
}
