package com.example.imadsheriff.droneapp;

import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.*;
import android.view.*;
import java.util.*;

public class OrderActivity extends AppCompatActivity implements View.OnClickListener, AdapterView.OnItemSelectedListener {
    private Button btnInstructions, btnPlaceOrder, btnCancelOrder, btnCheckOrder;
    private Spinner spnDrinks;
    private EditText editIP, editPortNum;
    private boolean isOrderPlaced = false;
    private String orderPlaced;//"--Select Drink--";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_order);
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
        if (orderPlaced.equals("--Select Drink--")) {
            showAlert("ALERT!", "Please select a drink");
            return;
        } else if (orderPlaced.equals("Coca Cola")) {
            showAlert("ALERT!", "This drink is not available. Please select another drink");
            return;
        }
        if (editIP.getText().toString().equals("")) {
            showAlert("ALERT!", "Please enter a valid IP Address");
            return;
        }
        if (editPortNum.getText().toString().equals("")) {
            showAlert("ALERT!", "Please enter a valid Port Number");
            return;
        }
        isOrderPlaced = true;
        showAlert("ALERT", "Your order is on its way.\nThanks for using our service!");
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
