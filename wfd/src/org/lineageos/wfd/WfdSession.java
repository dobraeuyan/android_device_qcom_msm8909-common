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
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ConnectionInfoListener;
import android.util.Log;

final class WfdSession implements RtspSource.Listener {
    private static final String TAG = "WfdProvider";

    interface Callback {
        void onConnected();
        void onFailed(String reason);
        void onClosed();
    }

    private final Context mContext;
    private final WifiP2pManager mManager;
    private final Channel mChannel;
    private final WifiP2pDevice mSink;
    private final Callback mCallback;

    private BroadcastReceiver mReceiver;
    private RtspSource mRtsp;
    private boolean mClosed;

    WfdSession(Context context, WifiP2pManager manager, Channel channel,
            WifiP2pDevice sink, Callback callback) {
        mContext = context;
        mManager = manager;
        mChannel = channel;
        mSink = sink;
        mCallback = callback;
    }

    void connect() {
        final IntentFilter filter =
                new IntentFilter(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        mReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context c, Intent intent) {
                final WifiP2pInfo info =
                        intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_INFO);
                if (info != null && info.groupFormed) {
                    onGroupFormed(info);
                }
            }
        };
        mContext.registerReceiver(mReceiver, filter);

        final WifiP2pConfig config = new WifiP2pConfig();
        config.deviceAddress = mSink.deviceAddress;
        config.wps = new WpsInfo();
        config.wps.setup = WpsInfo.PBC;

        config.groupOwnerIntent = 15;

        Log.i(TAG, "WfdSession: connecting to sink " + mSink.deviceAddress);
        mManager.connect(mChannel, config, new ActionListener() {
            @Override
            public void onSuccess() {
                Log.i(TAG, "WfdSession: P2P connect request accepted");
            }

            @Override
            public void onFailure(int reason) {
                fail("P2P connect failed, reason=" + reason);
            }
        });
    }

    private void onGroupFormed(final WifiP2pInfo info) {
        if (mRtsp != null || mClosed) {
            return;
        }

        if (!info.isGroupOwner) {

            fail("sink became Group Owner; source-as-client path not implemented (M2)");
            return;
        }
        final String bindAddr = info.groupOwnerAddress != null
                ? info.groupOwnerAddress.getHostAddress() : "0.0.0.0";
        Log.i(TAG, "WfdSession: P2P group formed, GO=" + bindAddr
                + "; starting RTSP source");
        mRtsp = new RtspSource(this);
        mRtsp.start(bindAddr, RtspSource.WFD_RTSP_PORT);
    }

    @Override
    public void onSessionReady(String videoFormats, int sinkRtpPort) {
        Log.i(TAG, "WfdSession: RTSP session up (PLAYING). Media path is M3. "
                + "sinkRtpPort=" + sinkRtpPort);
        mCallback.onConnected();

    }

    @Override
    public void onSessionError(String reason) {
        fail("RTSP: " + reason);
    }

    void close() {
        if (mClosed) {
            return;
        }
        mClosed = true;
        if (mRtsp != null) {
            mRtsp.stop();
            mRtsp = null;
        }
        if (mReceiver != null) {
            try {
                mContext.unregisterReceiver(mReceiver);
            } catch (IllegalArgumentException ignored) {
            }
            mReceiver = null;
        }
        if (mManager != null && mChannel != null) {
            mManager.removeGroup(mChannel, null);
        }
        mCallback.onClosed();
    }

    private void fail(String reason) {
        Log.e(TAG, "WfdSession: " + reason);
        if (!mClosed) {
            mCallback.onFailed(reason);
            close();
        }
    }
}
