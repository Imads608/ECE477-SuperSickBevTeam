package com.example.imadsheriff.droneapp;

import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.*;
import android.view.*;
import java.util.*;
import android.location.*;

public class OrderActivity extends AppCompatActivity implements View.OnClickListener, AdapterView.OnItemSelectedListener {
    private Button btnInstructions, btnPlaceOrder, btnCancelOrder, btnCheckOrder;
    private Spinner spnDrinks;
    private EditText editIP, editPortNum;
    private boolean isOrderPlaced = false;
    private String orderPlaced = "";//"--Select Drink--";
    private String latString = "";
    private String longString = "";
    private boolean isLocationOn = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_order);
        //while (latString == "" && longString == "") {
        getGPSCoordinates();
        //}
        init();
    }

    public void init() {
        btnInstructions = (Button) findViewById(R.id.btnInstructions);
        btnPlaceOrder = (Button) findViewById(R.id.btnPlaceOrder);
        btnCancelOrder = (Button) findViewById(R.id.btnCancelOrder);
        btnCheckOrder = (Button) findViewById(R.id.btnCheckOrder);
        spnDrinks = (Spinner) findViewById(R.id.spnDrinks);
        editIP = (EditText) findViewById(R.id.editIP);
        editPortNum = (EditText) findViewById(R.id.editPortNum);

        addItems(spnDrinks);


        btnInstructions.setOnClickListener(this);
        btnPlaceOrder.setOnClickListener(this);
        btnCheckOrder.setOnClickListener(this);
        btnCancelOrder.setOnClickListener(this);
        spnDrinks.setOnItemSelectedListener(this);
    }

    public void showInstructions() {
        String title = "Instructions";
        String message = "1 - Select an order from the drop down menu\n" +
                "2 - Once you are satisfied with your choice, place the order with the button located at the bottom.\n" +
                "3 - Placing the order will automatically check with the vender to see whether it is available and will alert you\n" +
                "4 - You will be notified once the order is placed. You are required to turn on your location so that the drone can deliver your beverage to you.\n" +
                "5 - Please wait patiently until your drink arrives.\n" +
                "6 - If you want to check what your current order is, you can click on the \"Check Current Order\" push button\n" +
                "7 - You cannot place more than one order at one time. Please wait until your current order arrives.\n" +
                "8 - Thanks for using our service!";

        showAlert(title, message);

    }

    public boolean checkLocationEnabled(LocationManager lm) {
        boolean gps_enabled = false;
        boolean network_enabled = false;

        try {
            gps_enabled = lm.isProviderEnabled(LocationManager.GPS_PROVIDER);
        } catch(Exception ex) {}

        try {
            network_enabled = lm.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
        } catch(Exception ex) {}

        if(!gps_enabled || !network_enabled) {
            return false;
        } else {
            isLocationOn = true;
        }

        return true;
    }

    public void setOrderPlaced(boolean value) {
        isOrderPlaced = value;
    }

    public void getGPSCoordinates() {
        // Acquire a reference to the system Location Manager
        LocationManager locationManager = (LocationManager) this.getSystemService(android.content.Context.LOCATION_SERVICE);
        if (checkLocationEnabled(locationManager) == false) {
            showAlert("Location Services", "Please turn on Location to order a drink");
            return;
        }
        // Define a listener that responds to location updates
        LocationListener locationListener = new LocationListener() {
            public void onLocationChanged(Location location) {
                // Called when a new location is found by the network location provider.
                latString = Double.toString(location.getLatitude());
                longString = Double.toString(location.getLongitude());
            }

            public void onStatusChanged(String provider, int status, Bundle extras) {}
            public void onProviderEnabled(String provider) {}
            public void onProviderDisabled(String provider) {}
        };
        // Register the listener with the Location Manager to receive location updates
        locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 0, 0, locationListener);
        locationManager.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 0, 0, locationListener);

        return;
    }

    public void clientSetup() {
        Client myClient = new Client(this, editIP.getText().toString(), Integer.parseInt(editPortNum.getText().toString()));
        String messageSend = "Check " + orderPlaced;
        String[] sendArray = {messageSend, latString, longString};
        myClient.execute(sendArray);

        /*
        if (!myClient.status.equals("") && myClient.status.equals("Connection Accepted")) {
            showAlert("Host Connection", "Connection Accepted");
        } else if (myClient.status.equals("In Stock")) {
            showAlert("ALERT", "Drink in stock. Order is on its way");
            isOrderPlaced = true;
        } else if (myClient.status.equals("Not in stock")) {
            showAlert("ALERT", "Drink not in stock. Order a different drink");
            isOrderPlaced = false;
        }*/
        return;
    }

    public void checkOrderStatus() {
        if (isOrderPlaced == false) {
            showAlert("Current Order", "You don't have any orders placed!");
            return;
        }
        showAlert("Current Order", "You ordered: " + orderPlaced);
    }

    public void cancelCurrentOrder() {
        if (isOrderPlaced == false) {
            showAlert("Denied", "You don't have an order placed to cancel");
            return;
        }

        //orderPlaced = "";
        isOrderPlaced = false;
        showAlert("ALERT", "Your order has been cancelled");
    }

    public void placeUserOrder() {
        if (isOrderPlaced == true) {
            showAlert("ALERT!", "You can only place one order at a time");
            return;
        }
        if (isLocationOn == false) {
            getGPSCoordinates();
            if (isLocationOn == false) {
                return;
            }
        }
        clientSetup();
    }

    public void showAlert(String title, String message) {

        AlertDialog.Builder builder = new AlertDialog.Builder(OrderActivity.this);
        builder.setCancelable(true);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                dialogInterface.cancel();
            }
        });

        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {

            }
        });
        builder.show();

        return;
    }

    public void addItems(Spinner spnDrinks) {
        List<String> categories = new ArrayList<String>();
        categories.add("--Select Drink--");
        categories.add("Coca Cola");
        categories.add("Sprite");
        categories.add("Root Beer");
        categories.add("Fanta");
        categories.add("Mello Yello");

        // Creating adapter for spinner
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, categories);

        // Drop down layout style - list view with radio button
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        // attaching data adapter to spinner
        spnDrinks.setAdapter(dataAdapter);
    }

    @Override
    public void onClick(View v) {

        if (v.getId() == R.id.btnInstructions) {
            showInstructions();
        } else if (v.getId() == R.id.btnCheckOrder) {
            checkOrderStatus();
        } else if (v.getId() == R.id.btnCancelOrder) {
            cancelCurrentOrder();
        } else if (v.getId() == R.id.btnPlaceOrder) {
            placeUserOrder();
        }

        return;
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        String selectedItem = parent.getSelectedItem().toString();
        orderPlaced = selectedItem;

        /*if (selectedItem != "--Select Drink--") {
            showAlert("ALERT!", "This drink is not available. Please select another drink");
        }*/
        return;
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {

        return;
    }
}
