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
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

final class RtspSource {
    private static final String TAG = "WfdProvider";

    static final int WFD_RTSP_PORT = 7236;
    private static final String USER_AGENT = "LineageOS-WFD/1.0";
    private static final String WFD_URL = "rtsp://localhost/wfd1.0";

    interface Listener {

        void onSessionReady(String videoFormats, int sinkRtpPort);
        void onSessionError(String reason);
    }

    private final Listener mListener;
    private ServerSocket mServer;
    private Socket mSink;
    private Thread mThread;
    private volatile boolean mStopped;

    private OutputStream mOut;
    private BufferedReader mIn;
    private int mNextCSeq = 1;

    private final Map<Integer, String> mPending = new HashMap<>();

    private String mSinkVideoFormats = "";
    private int mSinkRtpPort = -1;

    RtspSource(Listener listener) {
        mListener = listener;
    }

    void start(final String bindAddr, final int port) {
        mThread = new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    serve(bindAddr, port);
                } catch (IOException e) {
                    if (!mStopped) {
                        Log.e(TAG, "RtspSource: I/O error: " + e.getMessage());
                        mListener.onSessionError(e.getMessage());
                    }
                } finally {
                    closeQuietly();
                }
            }
        }, "wfd-rtsp-source");
        mThread.start();
    }

    void stop() {
        mStopped = true;
        closeQuietly();
        if (mThread != null) {
            mThread.interrupt();
        }
    }

    private void serve(String bindAddr, int port) throws IOException {
        mServer = new ServerSocket();
        mServer.setReuseAddress(true);
        mServer.bind(new InetSocketAddress(bindAddr, port));
        Log.i(TAG, "RtspSource: listening on " + bindAddr + ":" + port);

        mSink = mServer.accept();
        mSink.setTcpNoDelay(true);
        mOut = mSink.getOutputStream();
        mIn = new BufferedReader(new InputStreamReader(
                mSink.getInputStream(), StandardCharsets.US_ASCII));
        Log.i(TAG, "RtspSource: sink connected from " + mSink.getRemoteSocketAddress());

        sendM1();

        while (!mStopped) {
            RtspMessage msg = readMessage(mIn);
            if (msg == null) {
                break;
            }
            if (msg.isResponse) {
                handleResponse(msg);
            } else {
                handleRequest(msg);
            }
        }
    }

    private void sendM1() throws IOException {
        String req = "OPTIONS * RTSP/1.0\r\n"
                + commonHeaders(mNextCSeq)
                + "Require: org.wfa.wfd1.0\r\n\r\n";
        mPending.put(mNextCSeq, "M1");
        write(req);
        mNextCSeq++;
    }

    private void sendM3() throws IOException {
        String body = "wfd_content_protection\r\n"
                + "wfd_video_formats\r\n"
                + "wfd_audio_codecs\r\n"
                + "wfd_client_rtp_ports\r\n";
        String req = "GET_PARAMETER " + WFD_URL + " RTSP/1.0\r\n"
                + commonHeaders(mNextCSeq)
                + "Content-Type: text/parameters\r\n"
                + "Content-Length: " + body.length() + "\r\n\r\n"
                + body;
        mPending.put(mNextCSeq, "M3");
        write(req);
        mNextCSeq++;
    }

    private void sendM4() throws IOException {

        String video = mSinkVideoFormats.isEmpty()
                ? "00 00 02 02 00000040 00000000 00000000 00 0000 0000 00 none none"
                : mSinkVideoFormats;
        StringBuilder body = new StringBuilder();
        body.append("wfd_video_formats: ").append(video).append("\r\n");
        body.append("wfd_audio_codecs: AAC 00000001 00\r\n");
        body.append("wfd_presentation_URL: ")
                .append("rtsp://localhost/wfd1.0/streamid=0 none\r\n");
        body.append("wfd_client_rtp_ports: RTP/AVP/UDP;unicast ")
                .append(mSinkRtpPort > 0 ? mSinkRtpPort : 19000).append(" 0 mode=play\r\n");
        String req = "SET_PARAMETER " + WFD_URL + " RTSP/1.0\r\n"
                + commonHeaders(mNextCSeq)
                + "Content-Type: text/parameters\r\n"
                + "Content-Length: " + body.length() + "\r\n\r\n"
                + body;
        mPending.put(mNextCSeq, "M4");
        write(req);
        mNextCSeq++;
    }

    private void sendM5TriggerSetup() throws IOException {
        String body = "wfd_trigger_method: SETUP\r\n";
        String req = "SET_PARAMETER " + WFD_URL + " RTSP/1.0\r\n"
                + commonHeaders(mNextCSeq)
                + "Content-Type: text/parameters\r\n"
                + "Content-Length: " + body.length() + "\r\n\r\n"
                + body;
        mPending.put(mNextCSeq, "M5");
        write(req);
        mNextCSeq++;
    }

    private void handleResponse(RtspMessage msg) throws IOException {
        String which = mPending.remove(msg.cseq);
        if (which == null) {
            return;
        }
        if (msg.statusCode != 200) {
            mListener.onSessionError(which + " failed: " + msg.statusLine);
            return;
        }
        switch (which) {
            case "M1":
                sendM3();
                break;
            case "M3":
                parseSinkCapabilities(msg.body);
                sendM4();
                break;
            case "M4":
                sendM5TriggerSetup();
                break;
            case "M5":

                Log.i(TAG, "RtspSource: SETUP trigger acked; awaiting sink SETUP");
                break;
            default:
                break;
        }
    }

    private void handleRequest(RtspMessage msg) throws IOException {
        if ("OPTIONS".equals(msg.method)) {

            write("RTSP/1.0 200 OK\r\n"
                    + commonHeaders(msg.cseq)
                    + "Public: org.wfa.wfd1.0, GET_PARAMETER, SET_PARAMETER, "
                    + "SETUP, PLAY, PAUSE, TEARDOWN\r\n\r\n");
        } else if ("SETUP".equals(msg.method)) {

            int sinkRtp = parseClientRtpPort(msg.headers.get("transport"));
            if (sinkRtp > 0) {
                mSinkRtpPort = sinkRtp;
            }
            write("RTSP/1.0 200 OK\r\n"
                    + commonHeaders(msg.cseq)
                    + "Session: 1234ABCD;timeout=30\r\n"
                    + "Transport: RTP/AVP/UDP;unicast;client_port="
                    + (mSinkRtpPort > 0 ? mSinkRtpPort : 19000)
                    + ";server_port=19000-19001\r\n\r\n");
        } else if ("PLAY".equals(msg.method)) {

            write("RTSP/1.0 200 OK\r\n"
                    + commonHeaders(msg.cseq)
                    + "Session: 1234ABCD\r\n\r\n");
            Log.i(TAG, "RtspSource: session PLAYING; sinkRtpPort=" + mSinkRtpPort);
            mListener.onSessionReady(mSinkVideoFormats, mSinkRtpPort);
        } else if ("TEARDOWN".equals(msg.method)) {
            write("RTSP/1.0 200 OK\r\n" + commonHeaders(msg.cseq) + "\r\n");
            mListener.onSessionError("sink sent TEARDOWN");
            mStopped = true;
        } else {
            write("RTSP/1.0 200 OK\r\n" + commonHeaders(msg.cseq) + "\r\n");
        }
    }

    private void parseSinkCapabilities(String body) {
        if (body == null) {
            return;
        }
        for (String line : body.split("\r\n")) {
            int c = line.indexOf(':');
            if (c < 0) {
                continue;
            }
            String key = line.substring(0, c).trim();
            String val = line.substring(c + 1).trim();
            if ("wfd_video_formats".equals(key)) {
                mSinkVideoFormats = val;
            } else if ("wfd_client_rtp_ports".equals(key)) {
                mSinkRtpPort = parseWfdClientRtpPorts(val);
            }
        }
        Log.d(TAG, "RtspSource: sink caps parsed, rtpPort=" + mSinkRtpPort);
    }

    private static int parseWfdClientRtpPorts(String val) {
        try {
            String[] t = val.split("\\s+");
            for (int i = 0; i < t.length; i++) {
                if (t[i].startsWith("RTP/AVP") && i + 1 < t.length) {
                    return Integer.parseInt(t[i + 1]);
                }
            }
        } catch (NumberFormatException ignored) {
        }
        return -1;
    }

    private static int parseClientRtpPort(String transport) {
        if (transport == null) {
            return -1;
        }
        int i = transport.indexOf("client_port=");
        if (i < 0) {
            return -1;
        }
        String rest = transport.substring(i + "client_port=".length());
        String num = rest.split("[-;\\s]")[0];
        try {
            return Integer.parseInt(num);
        } catch (NumberFormatException e) {
            return -1;
        }
    }

    private String commonHeaders(int cseq) {
        return "Server: " + USER_AGENT + "\r\n" + "CSeq: " + cseq + "\r\n";
    }

    private void write(String s) throws IOException {
        mOut.write(s.getBytes(StandardCharsets.US_ASCII));
        mOut.flush();
    }

    private void closeQuietly() {
        try { if (mSink != null) mSink.close(); } catch (IOException ignored) {}
        try { if (mServer != null) mServer.close(); } catch (IOException ignored) {}
    }

    static final class RtspMessage {
        boolean isResponse;
        String method;
        String statusLine;
        int statusCode;
        int cseq;
        final Map<String, String> headers = new HashMap<>();
        String body;
    }

    static RtspMessage readMessage(BufferedReader in) throws IOException {
        String startLine = in.readLine();
        if (startLine == null) {
            return null;
        }
        while (startLine.isEmpty()) {
            startLine = in.readLine();
            if (startLine == null) {
                return null;
            }
        }
        RtspMessage m = new RtspMessage();
        if (startLine.startsWith("RTSP/")) {
            m.isResponse = true;
            m.statusLine = startLine;
            try {
                m.statusCode = Integer.parseInt(startLine.split("\\s+")[1]);
            } catch (Exception e) {
                m.statusCode = -1;
            }
        } else {
            m.isResponse = false;
            m.method = startLine.split("\\s+")[0];
        }
        String line;
        int contentLength = 0;
        while ((line = in.readLine()) != null && !line.isEmpty()) {
            int c = line.indexOf(':');
            if (c < 0) {
                continue;
            }
            String key = line.substring(0, c).trim().toLowerCase();
            String val = line.substring(c + 1).trim();
            m.headers.put(key, val);
            if ("cseq".equals(key)) {
                try { m.cseq = Integer.parseInt(val); } catch (NumberFormatException ignored) {}
            } else if ("content-length".equals(key)) {
                try { contentLength = Integer.parseInt(val); } catch (NumberFormatException ignored) {}
            }
        }
        if (contentLength > 0) {
            char[] buf = new char[contentLength];
            int read = 0;
            while (read < contentLength) {
                int n = in.read(buf, read, contentLength - read);
                if (n < 0) {
                    break;
                }
                read += n;
            }
            m.body = new String(buf, 0, read);
        }
        return m;
    }
}
