package org.test.dexvmp;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;


public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        TextView tv = findViewById(R.id.text_view);
        tv.setText("5 + 8 = " +  MyAdd(5, 8));

        //onCreate2(savedInstanceState);

    }

    protected void onCreate2(Bundle savedInstanceState) {
        ProxyApp.interface1(new Object[] {1001, this, savedInstanceState});
    }

    public int MyAdd(int n1, int n2) {
        return ProxyApp.interface1(new Object[] {1000, this, n1, n2});
    }
    
}