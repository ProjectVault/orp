/** @file BulkEncryption.java */
/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
package orp.demo.bulkencryption;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;

import orp.orp.Endpoint;
import orp.orp.Outbound;
import orp.orp.Session;
import orp.orp.SessionManager;
import orp.orp.TIDL;

public class BulkEncryption {
    public static final byte[] bulkencryption_hash =
            { 0x62, 0x75, 0x6C, 0x6B, 0x65, 0x6E, 0x63, 0x72,
              0x79, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0xa,  0x0,
              0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
              0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0
            };
    public static final short bulkencryption_epnum = 0x1337;

    private final SessionManager sessionManager;
    private Session session = null;

    private BulkEncryption(SessionManager in_sessionManager) {
        sessionManager = in_sessionManager;
    }

    private void log(String message) {
        Log.d("BulkEncryption", message);
    }

    public static BulkEncryption getBulkEncryption(SessionManager in_sessionManager) {
        BulkEncryption ret = new BulkEncryption(in_sessionManager);

        try {
            Endpoint ep = new Endpoint(bulkencryption_hash, (short) bulkencryption_epnum);
            SessionManager.PreSession preSession = ret.sessionManager.newSession(ep);
            ret.session = preSession.getSession();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return ret;
    }

    public boolean DoLogout() throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            BulkEncryptionCommand.Serialize(command, BulkEncryptionCommand.BEC_LOGOUT);

            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return false;
            ByteBuffer response = ByteBuffer.wrap(session.read());

            BulkEncryptionResponse response_type = BulkEncryptionResponse.Deserialize(response);
            if (BulkEncryptionResponse.BER_GOODBYE != response_type) return false;
            log("Logged out");
            return true;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return false;
    }

    public GenDHPair_Res DoGenDHPair(GenDHPair_Cmd cmd) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            BulkEncryptionCommand.Serialize(command, BulkEncryptionCommand.BEC_GEN_DHPAIR);
            GenDHPair_Cmd.Serialize(command, cmd, 1);

            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());

            BulkEncryptionResponse response_type = BulkEncryptionResponse.Deserialize(response);
            if (BulkEncryptionResponse.BER_DHPAIR != response_type) return null;
            GenDHPair_Res res = GenDHPair_Res.Deserialize(response, 1);
            log("Generated DH pair");
            return res;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public DHExchange_Res DoDHExchange(DHExchange_Cmd cmd) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            BulkEncryptionCommand.Serialize(command, BulkEncryptionCommand.BEC_DH_EXCHANGE);
            DHExchange_Cmd.Serialize(command, cmd, 1);

            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());

            BulkEncryptionResponse response_type = BulkEncryptionResponse.Deserialize(response);
            if (BulkEncryptionResponse.BER_DH_EXCHANGED != response_type) return null;
            DHExchange_Res res = DHExchange_Res.Deserialize(response, 1);
            log("Performed DH exchange");
            return res;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public GcmInit_Res DoGcmInit(GcmInit_Cmd cmd) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            BulkEncryptionCommand.Serialize(command, BulkEncryptionCommand.BEC_GCM_INIT);
            GcmInit_Cmd.Serialize(command, cmd, 1);

            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());

            BulkEncryptionResponse response_type = BulkEncryptionResponse.Deserialize(response);
            if (BulkEncryptionResponse.BER_GCM_READY != response_type) return null;
            GcmInit_Res res = GcmInit_Res.Deserialize(response, 1);
            log("GCM initialized");
            return res;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public GcmDoBlock_Res DoGcmBlock(GcmDoBlock_Cmd cmd) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            BulkEncryptionCommand.Serialize(command, BulkEncryptionCommand.BEC_GCM_DOBLOCK);
            GcmDoBlock_Cmd.Serialize(command, cmd, 1);

            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());

            BulkEncryptionResponse response_type = BulkEncryptionResponse.Deserialize(response);
            if (BulkEncryptionResponse.BER_GCM_BLOCKS != response_type) return null;
            GcmDoBlock_Res res = GcmDoBlock_Res.Deserialize(response, 1);
            log("GCM transform performed");
            return res;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }
}
