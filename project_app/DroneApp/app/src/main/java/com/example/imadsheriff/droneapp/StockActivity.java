package com.example.imadsheriff.droneapp;

import android.content.DialogInterface;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.*;
import android.view.*;

import java.util.ArrayList;
import java.util.List;

import pl.droidsonroids.gif.GifImageView;

public class StockActivity extends AppCompatActivity implements View.OnClickListener, AdapterView.OnItemSelectedListener {
    private Button btnSend;
    private Button btnExit;
    private Spinner spnChoose;
    private EditText editCode;
    private EditText editAmount;
    private GifImageView gifViewDrone;
    private TextView txtLoading2;
    private TextView txtCode;
    private TextView txtSelect;
    private TextView txtAdd;

    private String code = "608123";
    private String chosenDrink = "";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_stock);
        init();
    }

    public void clientSetup(String messageSend) {
        setLoadingScreen();
        Client myClient = new Client(this, true, editAmount.getText().toString(), chosenDrink, "192.168.4.1", 80);
        //String messageSend = "Check " + orderPlaced;
        String[] sendArray = {messageSend};
        myClient.execute(sendArray);

        return;
    }

    public void setLoadingScreen() {
        btnSend.setVisibility(View.GONE);
        btnExit.setVisibility(View.GONE);
        spnChoose.setVisibility(View.GONE);
        editCode.setVisibility(View.GONE);
        editAmount.setVisibility(View.GONE);
        txtLoading2.setVisibility(View.VISIBLE);
        txtCode.setVisibility(View.GONE);
        txtSelect.setVisibility(View.GONE);
        txtAdd.setVisibility(View.GONE);


        gifViewDrone.setVisibility(View.VISIBLE);
    }

    public void finishLoadingScreen() {
        btnSend.setVisibility(View.VISIBLE);
        btnExit.setVisibility(View.VISIBLE);
        spnChoose.setVisibility(View.VISIBLE);
        editCode.setVisibility(View.VISIBLE);
        editAmount.setVisibility(View.VISIBLE);
        txtLoading2.setVisibility(View.GONE);
        txtCode.setVisibility(View.VISIBLE);
        txtSelect.setVisibility(View.VISIBLE);
        txtAdd.setVisibility(View.VISIBLE);


        gifViewDrone.setVisibility(View.GONE);
    }

    public void showAlert(String title, String message) {

        AlertDialog.Builder builder = new AlertDialog.Builder(StockActivity.this);
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

    public void init() {
        btnSend = (Button) findViewById(R.id.btnSend);
        btnExit = (Button) findViewById(R.id.btnExit);
        spnChoose = (Spinner) findViewById(R.id.spnChoose);
        editCode = (EditText) findViewById(R.id.editCode);
        editAmount = (EditText) findViewById(R.id.editAmount);
        gifViewDrone = (GifImageView) findViewById(R.id.gifViewDrone);
        txtCode = (TextView) findViewById(R.id.txtCode);
        txtSelect = (TextView) findViewById(R.id.txtSelect);
        txtAdd = (TextView) findViewById(R.id.txtAdd);
        txtLoading2 = (TextView) findViewById(R.id.txtView_Loading2);
        txtLoading2.setVisibility(View.GONE);
        addItems(spnChoose);


        btnSend.setOnClickListener(this);
        btnExit.setOnClickListener(this);
        spnChoose.setOnItemSelectedListener(this);
    }

    public void sendUpdate() {
        String enteredCode = "";
        String send = "";

        if (!editCode.getText().toString().equals(code)) {
            showAlert("Denied", "Invalid code");
            return;
        } else if (chosenDrink.equals("--Select Drink--") || chosenDrink.equals("")) {
            showAlert("Denied", "Please select a drink");
            return;
        } else if (editAmount.getText().toString().equals("")) {
            showAlert("Denied", "Please select an amount to update");
            return;
        }
        send = "Calc Update";
        clientSetup(send);

    }

    public void exitActivity() {
        finish();
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

        if (v.getId() == R.id.btnSend) {
            sendUpdate();
        } else if (v.getId() == R.id.btnExit) {
            exitActivity();//checkOrderStatus();
        }

        return;
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        String selectedItem = parent.getSelectedItem().toString();
        chosenDrink = selectedItem;
        //orderPlaced = selectedItem;

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
