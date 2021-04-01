/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package com.mycompany.vibrain.utils;

import communication.CoAPConnection;
import format.CoAPMessage;
import ipsoprofile.IPSOMessage;
import ipsoprofile.Methods;
import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;

/**
 *
 * @author Hp
 */
public class Operaciones {

    private static int findFreePort()
            throws IOException {

        int port;
        ServerSocket server = new ServerSocket(0);
        port = server.getLocalPort();

        return port;
    }
    //ENCIENDE LA LAMPARA

    public static String setltOn() {

        String on = "";
        int port = 0;
        try {
            port = findFreePort();
        } catch (IOException ex) {
        }

        try {

            CoAPConnection c = new CoAPConnection(port, 1000);

            c.sendMessage(new CoAPMessage(CoAPMessage.PUT_OP, CoAPMessage.MESSAGE_TYPE_CON,
                    new IPSOMessage("/lt/on 1", null, Methods.PUT)),
                    InetAddress.getByName("2001:720:1710:11::2"), 1234);

            String status = new String(c.receiveMessage().getPayload()).trim();
            //infotextarea.setText(infotextarea.getText()+"Switched ON Command SENT:"+ loctext);   
            //gpstext.setText(loctext);

            c.close();
            setDim("255");
            return status;

            //return loctext;

        } catch (IOException ex) {
        }
        return on;
    }
    //APAGA LA LAMPARA

    public static String setltOff() {

        String off = "";
        int port = 0;
        try {
            port = findFreePort();
        } catch (IOException ex) {
        }

        try {

            CoAPConnection c = new CoAPConnection(port, 1000);

            c.sendMessage(new CoAPMessage(CoAPMessage.PUT_OP, CoAPMessage.MESSAGE_TYPE_CON,
                    new IPSOMessage("/lt/on 0", null, Methods.PUT)),
                    InetAddress.getByName("2001:720:1710:11::2"), 1234);

            String status = new String(c.receiveMessage().getPayload()).trim();
            //infotextarea.setText(infotextarea.getText()+"Switched ON Command SENT:"+ loctext);   
            //gpstext.setText(loctext);

            c.close();
            setDim("-255");
            return status;

            //return loctext;

        } catch (IOException ex) {
        }
        return off;
    }

    public static String setDim(String value) {
        String dim = "";
        int port = 0;
        try {
            port = findFreePort();
        } catch (IOException ex) {
        }

        try {

            CoAPConnection c = new CoAPConnection(port, 1000);
            c.sendMessage(new CoAPMessage(CoAPMessage.PUT_OP, CoAPMessage.MESSAGE_TYPE_CON,
                    new IPSOMessage("/lt/dim "+value, null, Methods.PUT)),
                    InetAddress.getByName("2001:720:1710:11::2"), 1234);

            String dimset = new String(c.receiveMessage().getPayload()).trim();
            //infotextarea.setText(infotextarea.getText()+"Switched ON Command SENT:"+ loctext);   
            //gpstext.setText(loctext);

            c.close();

            return dimset;

            //return loctext;

        } catch (IOException ex) {
        }
        return dim;
    }
}
