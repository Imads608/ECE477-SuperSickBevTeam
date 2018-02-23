package com.example.imadsheriff.droneapp;

import android.os.AsyncTask;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.UnknownHostException;

public class Client extends AsyncTask<String, Void, Void> {

    private String dstAddress;
    private int dstPort;
    private String response = "";
    private String status = "";
    private String decision = "";

    //TextView textResponse;

    Client(String addr, int port) {
        dstAddress = addr;
        dstPort = port;
        //this.textResponse = textResponse;
    }

    @Override
    protected Void doInBackground(String... pParams) {

        Socket socket = null;
        String request = pParams[0];
        String latString = pParams[1];
        String longString = pParams[2];
        String gpsCoordinates = latString + ", " + longString;

        try {
            socket = new Socket(dstAddress, dstPort);
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream(1024);
            byte[] buffer = new byte[1024];
            int bytesRead;
            InputStream inputStream = socket.getInputStream();
            DataOutputStream dOut = new DataOutputStream(socket.getOutputStream());

            while ((bytesRead = inputStream.read(buffer)) != -1) {
                byteArrayOutputStream.write(buffer, 0, bytesRead);
                response = byteArrayOutputStream.toString("UTF-8");
                buffer = new byte[1024];
                byteArrayOutputStream = new ByteArrayOutputStream(1024);
                // Waits for server to acknowledge client connection
                if (response.contains("Hello Client")) {
                    status = "Connection Accepted";

                    // Send drink request to server
                    dOut.writeUTF(request);
                    dOut.flush();

                    // Wait for server to check if drink is available
                } else if (response.contains("Not Available")) {
                    status = "Not in stock";
                } else if (response.contains("Available")) {
                    status = "In stock";

                    // Send GPS Coordinates
                    dOut.writeUTF(gpsCoordinates);

                  // Wait for server to accept GPS coordinates
                } else if (response.contains("Delivering Order")) {
                    status = "Delivering";
                }
            }

            // Done with communication

        } catch (UnknownHostException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            response = "UnknownHostException: " + e.toString();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            response = "IOException: " + e.toString();
        } finally {
            if (socket != null) {
                try {
                    socket.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        //textResponse.setText(response);
        decision = status;
        super.onPostExecute(result);
    }

    public String getDecision() {
        return decision;
    }

}