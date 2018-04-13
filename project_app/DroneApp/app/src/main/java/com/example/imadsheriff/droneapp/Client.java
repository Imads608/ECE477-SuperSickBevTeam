package com.example.imadsheriff.droneapp;

import android.annotation.SuppressLint;
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
import java.net.InetSocketAddress;

public class Client extends AsyncTask<String, Void, Void> {

    private String dstAddress;
    private int dstPort;
    private String status = "";
    private Boolean devOn = false;
    private String updateStock = "";
    private String updateDrink = "";

    //TextView textResponse;
    @SuppressLint("StaticFieldLeak")
    public OrderActivity myOrderActivity;
    public StockActivity myStockActivity;

    Client(OrderActivity a, String addr, int port) {
        this.myOrderActivity = a;
        dstAddress = addr;
        dstPort = port;
        //this.textResponse = textResponse;
    }

    Client(StockActivity a, Boolean devUpdate, String numUpdate, String drinkName, String addr, int port) {
        this.myStockActivity = a;
        devOn = devUpdate;
        updateStock = numUpdate;
        updateDrink = drinkName;
        dstAddress = addr;
        dstPort = port;
        //this.textResponse = textResponse;
    }


    @Override
    protected Void doInBackground(String... pParams) {

        Socket socket = null;
        String request = "";
        String latString = "";
        String longString = "";
        String gpsCoordinates = "";
        String response = "";

        if (devOn == false) {
            request = pParams[0];
            latString = pParams[1];
            longString = pParams[2];
            gpsCoordinates = latString + ", " + longString;
            response = "";
        } else {
            request = pParams[0];
        }

        try {
            socket = new Socket();
            socket.connect(new InetSocketAddress(dstAddress, dstPort), 5000);
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

                    // Send drink request to server or start stock update
                    dOut.writeUTF(request);
                    dOut.flush();

                    // Wait for server to check if drink is available
                } else if (response.contains("Not Available")) {
                    status = "Not in stock";
                } else if (response.contains("Available")) {
                    status = "In stock";

                    // Send GPS Coordinates
                    dOut.writeUTF(gpsCoordinates);
                    dOut.flush();

                    // Wait for server to accept GPS coordinates
                } else if (response.contains("Delivering Order")) {
                    status = "Delivering";
                } else if (response.contains("Order Cancelled")) {
                    status = "Order Cancelled";
                } else if (response.contains("Got Initial Request")) {
                    status = "Request in progress";

                    // Send number of drinks to update
                    String toSend = "Calc " + updateStock;
                    dOut.writeUTF(toSend);
                    dOut.flush();
                } else if (response.contains("Got Number to order")) {
                    status = "Request in progress";

                    // Send drink name to update
                    String toSend2 = "Calc " + updateDrink;
                    dOut.writeUTF(toSend2);
                    dOut.flush();
                } else if (response.contains("Got Drink Name")) {
                    status = "Stock Update finished";
                } else if (response.contains("Got Stock Request")) {
                    status = "Stock request in progress";

                    dOut.writeUTF("Check " + updateDrink);
                    dOut.flush();
                } else if (response.contains("Stock is ")) {
                    status = response;
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
        if (devOn == false) {
            myOrderActivity.finishLoadingScreen();
        } else {
            myStockActivity.finishLoadingScreen();
        }
        if (status == "Delivering") {
            myOrderActivity.showAlert("Order Update", "Your Order is on its way");
            myOrderActivity.setOrderPlaced(true);
        } else if (status == "Not in stock") {
            myOrderActivity.showAlert("Order Update", "Out of Stock. Please select a different drink");
            myOrderActivity.setOrderPlaced(false);
        } else if (status == "Order Cancelled") {
            myOrderActivity.showAlert("Order Update", "Your order has been cancelled");
            myOrderActivity.setOrderPlaced(false);
        } else if (status == "Stock Update finished") {
            myStockActivity.showAlert("Stock Update", "Successfully updated stock");
            devOn = false;
        } else if (status.contains("Stock is ")) {
            myStockActivity.showAlert("Current Stock", status);
            devOn = false;
        } else {
            if (devOn == false) {
                myOrderActivity.showAlert("Connection Status", "Could not get a connection with drone. Please try again later");
                myOrderActivity.setOrderPlaced(false);
            } else if (devOn == true) {
                myStockActivity.showAlert("Connection Status", "Could not get a connection with drone. Please try again later");
                devOn = false;
            }
        }

        super.onPostExecute(result);
    }
}