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

import android.util.Log;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

final class RtspSelfTest {
    private static final String TAG = "WfdProvider";
    private static final int TEST_PORT = 27236;

    static void run() {
        final AtomicBoolean sessionReady = new AtomicBoolean(false);
        final CountDownLatch done = new CountDownLatch(1);
        final StringBuilder failure = new StringBuilder();

        final RtspSource source = new RtspSource(new RtspSource.Listener() {
            @Override
            public void onSessionReady(String videoFormats, int sinkRtpPort) {
                Log.i(TAG, "selfTest: source reached PLAYING, sinkRtpPort=" + sinkRtpPort);
                sessionReady.set(sinkRtpPort > 0);
                if (sinkRtpPort <= 0) {
                    failure.append("no sink RTP port negotiated; ");
                }
                done.countDown();
            }

            @Override
            public void onSessionError(String reason) {
                failure.append("source error: ").append(reason).append("; ");
                done.countDown();
            }
        });
        source.start("127.0.0.1", TEST_PORT);

        Thread sink = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    mockSink(failure);
                } catch (Exception e) {
                    failure.append("sink exception: ").append(e).append("; ");
                    done.countDown();
                }
            }
        }, "wfd-mock-sink");
        sink.start();

        boolean finished;
        try {
            finished = done.await(8, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            finished = false;
        }
        source.stop();

        if (finished && sessionReady.get() && failure.length() == 0) {
            Log.i(TAG, "RtspSource selfTest: PASS (M1-M7 handshake completed)");
        } else {
            Log.e(TAG, "RtspSource selfTest: FAIL ("
                    + (failure.length() == 0 ? "timeout" : failure.toString()) + ")");
        }
    }

    private static void mockSink(StringBuilder failure) throws Exception {

        Thread.sleep(300);
        Socket s = new Socket();
        s.connect(new InetSocketAddress("127.0.0.1", TEST_PORT), 3000);
        s.setTcpNoDelay(true);
        final OutputStream out = s.getOutputStream();
        final BufferedReader in = new BufferedReader(new InputStreamReader(
                s.getInputStream(), StandardCharsets.US_ASCII));

        int myCseq = 100;
        int optionsCseq = -1, setupCseq = -1, playCseq = -1;
        boolean sentOptions = false, sentSetup = false, sentPlay = false;

        while (true) {
            RtspSource.RtspMessage m = RtspSource.readMessage(in);
            if (m == null) {
                break;
            }
            if (!m.isResponse) {

                if ("OPTIONS".equals(m.method)) {

                    reply(out, m.cseq,
                            "Public: org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER\r\n");
                    if (!sentOptions) {

                        optionsCseq = myCseq++;
                        send(out, "OPTIONS * RTSP/1.0\r\nCSeq: " + optionsCseq
                                + "\r\nRequire: org.wfa.wfd1.0\r\n\r\n");
                        sentOptions = true;
                    }
                } else if ("GET_PARAMETER".equals(m.method)) {

                    String body = "wfd_content_protection: none\r\n"
                            + "wfd_video_formats: 00 00 02 02 00000040 00000000 00000000 "
                            + "00 0000 0000 00 none none\r\n"
                            + "wfd_audio_codecs: AAC 00000001 00\r\n"
                            + "wfd_client_rtp_ports: RTP/AVP/UDP;unicast 19000 0 mode=play\r\n";
                    replyBody(out, m.cseq, body);
                } else if ("SET_PARAMETER".equals(m.method)) {
                    boolean isTrigger = m.body != null && m.body.contains("wfd_trigger_method");
                    reply(out, m.cseq, "");
                    if (isTrigger && !sentSetup) {

                        setupCseq = myCseq++;
                        send(out, "SETUP rtsp://localhost/wfd1.0/streamid=0 RTSP/1.0\r\nCSeq: "
                                + setupCseq + "\r\nTransport: RTP/AVP/UDP;unicast;"
                                + "client_port=19000-19001\r\n\r\n");
                        sentSetup = true;
                    }
                } else {
                    reply(out, m.cseq, "");
                }
            } else {

                if (m.cseq == setupCseq && !sentPlay) {

                    playCseq = myCseq++;
                    send(out, "PLAY rtsp://localhost/wfd1.0/streamid=0 RTSP/1.0\r\nCSeq: "
                            + playCseq + "\r\nSession: 1234ABCD\r\n\r\n");
                    sentPlay = true;
                } else if (m.cseq == playCseq) {

                    Log.i(TAG, "selfTest: sink got PLAY 200; handshake OK");
                    break;
                }
            }
        }
        s.close();
    }

    private static void reply(OutputStream out, int cseq, String extra) throws Exception {
        send(out, "RTSP/1.0 200 OK\r\nCSeq: " + cseq + "\r\n" + extra + "\r\n");
    }

    private static void replyBody(OutputStream out, int cseq, String body) throws Exception {
        send(out, "RTSP/1.0 200 OK\r\nCSeq: " + cseq
                + "\r\nContent-Type: text/parameters\r\nContent-Length: "
                + body.length() + "\r\n\r\n" + body);
    }

    private static void send(OutputStream out, String s) throws Exception {
        out.write(s.getBytes(StandardCharsets.US_ASCII));
        out.flush();
    }
}
