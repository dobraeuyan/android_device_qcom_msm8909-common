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

import com.android.media.remotedisplay.RemoteDisplay;
import com.android.media.remotedisplay.RemoteDisplayProvider;

import android.app.Service;
import android.content.Intent;
import android.net.wifi.p2p.WifiP2pDevice;
import android.os.Handler;
import android.os.IBinder;
import android.text.TextUtils;
import android.util.Log;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class WfdProviderService extends Service {
    private static final String TAG = "WfdProvider";

    static final String ACTION_SELFTEST = "org.lineageos.wfd.SELFTEST";

    private WfdProvider mProvider;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && ACTION_SELFTEST.equals(intent.getAction())) {
            Log.i(TAG, "Starting RtspSource loopback self-test");
            new Thread(new Runnable() {
                @Override
                public void run() {
                    RtspSelfTest.run();
                }
            }, "wfd-selftest").start();
        }
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (intent.getAction().equals(RemoteDisplayProvider.SERVICE_INTERFACE)) {
            Log.d(TAG, "onBind: framework requested the RemoteDisplayProvider binder");
            if (mProvider == null) {
                mProvider = new WfdProvider();
            }
            return mProvider.getBinder();
        }
        return null;
    }

    final class WfdProvider extends RemoteDisplayProvider implements P2pDiscovery.Listener {
        private final Handler mHandler;
        private final P2pDiscovery mDiscovery;

        private final Map<String, RemoteDisplay> mDisplays = new HashMap<>();

        private final Map<String, WifiP2pDevice> mSinkDevices = new HashMap<>();
        private boolean mDiscovering;
        private WfdSession mSession;

        WfdProvider() {
            super(WfdProviderService.this);
            mHandler = new Handler(getMainLooper());
            mDiscovery = new P2pDiscovery(WfdProviderService.this, this);
        }

        @Override
        public void onDiscoveryModeChanged(int mode) {
            Log.d(TAG, "onDiscoveryModeChanged: mode=" + mode);
            if (mode != DISCOVERY_MODE_NONE) {
                if (!mDiscovering) {
                    mDiscovering = true;
                    if (mDiscovery.isSupported()) {
                        mDiscovery.start();
                    } else {
                        Log.w(TAG, "Wi-Fi Direct unsupported; no WFD sinks can be found");
                    }
                }
            } else {
                if (mDiscovering) {
                    mDiscovering = false;
                    mDiscovery.stop();

                    clearDisplays();
                }
            }
        }

        @Override
        public void onP2pStateChanged(boolean enabled) {
            if (!enabled) {
                Log.w(TAG, "P2P disabled by framework; clearing sink routes");
                clearDisplays();
            }
        }

        @Override
        public void onSinksChanged(List<WifiP2pDevice> sinks) {

            final Map<String, RemoteDisplay> stale = new HashMap<>(mDisplays);
            for (WifiP2pDevice d : sinks) {
                final String addr = d.deviceAddress;
                if (TextUtils.isEmpty(addr)) {
                    continue;
                }
                stale.remove(addr);
                mSinkDevices.put(addr, d);
                RemoteDisplay display = mDisplays.get(addr);
                final String name = TextUtils.isEmpty(d.deviceName) ? addr : d.deviceName;
                if (display == null) {
                    display = new RemoteDisplay(addr, name);
                    display.setDescription("Wi-Fi Display");
                    display.setStatus(RemoteDisplay.STATUS_AVAILABLE);
                    mDisplays.put(addr, display);
                    addDisplay(display);
                    Log.i(TAG, "WFD sink found: " + name + " (" + addr + ")");
                } else if (!name.equals(display.getName())) {
                    display.setName(name);
                    updateDisplay(display);
                }
            }

            for (Map.Entry<String, RemoteDisplay> e : stale.entrySet()) {
                removeDisplay(e.getValue());
                mDisplays.remove(e.getKey());
                mSinkDevices.remove(e.getKey());
                Log.i(TAG, "WFD sink lost: " + e.getKey());
            }
        }

        @Override
        public void onConnect(final RemoteDisplay display) {
            Log.d(TAG, "onConnect: id=" + display.getId());
            final WifiP2pDevice sink = mSinkDevices.get(display.getId());
            if (sink == null || !mDiscovery.isSupported()
                    || mDiscovery.getChannel() == null) {
                Log.w(TAG, "onConnect: no backing sink/channel for " + display.getId());
                display.setStatus(RemoteDisplay.STATUS_NOT_AVAILABLE);
                updateDisplay(display);
                return;
            }
            if (mSession != null) {
                mSession.close();
                mSession = null;
            }
            display.setStatus(RemoteDisplay.STATUS_CONNECTING);
            updateDisplay(display);

            mSession = new WfdSession(WfdProviderService.this, mDiscovery.getManager(),
                    mDiscovery.getChannel(), sink, new WfdSession.Callback() {
                @Override
                public void onConnected() {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            display.setStatus(RemoteDisplay.STATUS_CONNECTED);
                            updateDisplay(display);
                        }
                    });
                }

                @Override
                public void onFailed(final String reason) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            display.setStatus(RemoteDisplay.STATUS_AVAILABLE);
                            updateDisplay(display);
                        }
                    });
                }

                @Override
                public void onClosed() {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (display.getStatus() != RemoteDisplay.STATUS_AVAILABLE) {
                                display.setStatus(RemoteDisplay.STATUS_AVAILABLE);
                                updateDisplay(display);
                            }
                        }
                    });
                }
            });
            mSession.connect();
        }

        @Override
        public void onDisconnect(RemoteDisplay display) {
            Log.d(TAG, "onDisconnect: id=" + display.getId());
            if (mSession != null) {
                mSession.close();
                mSession = null;
            }
            display.setStatus(RemoteDisplay.STATUS_AVAILABLE);
            updateDisplay(display);
        }

        private void clearDisplays() {
            for (RemoteDisplay display : mDisplays.values()) {
                removeDisplay(display);
            }
            mDisplays.clear();
        }
    }
}
