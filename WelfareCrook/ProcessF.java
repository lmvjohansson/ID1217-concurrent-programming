package WelfareCrook;

import java.io.*;
import java.net.*;
import java.util.*;

/*
* Algorithm to solve Welfare Crook Problem
* Each process acts as a server to receive the lists from the other two processes, one name at a time.
* Each process acts as a client to the two other processes also, and send over their lists, when the list ends an empty string ("") is sent to signal end of file.
* Each process spawn four threads to ensure no deadlock, two threads for client processes, two threads for server processes.
* If a name is present in both the received list and the server's list then it is added to a list of common names, these lists are individual to the server threads.
* When the two other processes has sent their lists and common names are found and placed in the two common names lists the main thread filters out the names present in both lists.
* These names are present in all three lists and are the final result. So each process reaches the result individually, no result is passed between the processes.
* */

public class ProcessF {
    public static void main(String[] args) {
        String fileName = "process_F_data.txt"; // Text file with list of names
        List<String> namesList = new ArrayList<>();

        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) { // Read text file and add each name as a string in a list
            String line;
            while ((line = br.readLine()) != null) {
                namesList.add(line);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        new Thread(new ClientG(5002, namesList)).start(); // Start two client threads that attempt to connect to the other processes and send the list to them
        new Thread(new ClientG(5003, namesList)).start();

        List<String> commonNamesFirstConnection = new ArrayList<>(); // Two lists for storing names present in namesList and the names sent by another process
        List<String> commonNamesSecondConnection = new ArrayList<>();
        List<Thread> serverThreads = new ArrayList<>(); // List of threads for easy join at the end

        try (ServerSocket welcomeSocket = new ServerSocket(5001)) { // Attempt to open server socket and listen to port 5001
            System.out.println("Process F listening for incoming connections...");

            for (int i = 0; i < 2; i++) { // Handle the two connections from the other processes
                Socket connectionSocket = welcomeSocket.accept(); // Record the socket of the communication with the client
                System.out.println("Process F received connection");

                Server server;
                if (i == 0) { // Two separate server runnable objects so each thread has their own common names list
                    server = new Server(namesList, connectionSocket, commonNamesFirstConnection);
                } else {
                    server = new Server(namesList, connectionSocket, commonNamesSecondConnection);
                }

                Thread serverThread = new Thread(server); // Thread created, started and added to the list of threads
                serverThread.start();
                serverThreads.add(serverThread);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        for (Thread thread : serverThreads) { // For each thread in thread list, join them
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        commonNamesFirstConnection.retainAll(commonNamesSecondConnection); // Save only the names present in both common names lists, these names are present in all 3 lists.

        System.out.println("Process F result");
        System.out.println(commonNamesFirstConnection);
    }
}

class Server implements Runnable { // Class that represents running this process as a server
    List<String> namesList;
    Socket connectionSocket;
    private List<String> commonNames;

    Server (List<String> namesList, Socket connectionSocket, List<String> commonNames) {
        this.namesList = namesList;
        this.connectionSocket = connectionSocket;
        this.commonNames = commonNames;
    }

    @Override
    public void run() {
        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream())); // Attempt to read from client
            String name;
            while ((name = in.readLine()) != null) { // Reach each line and add the name to the list
                if (namesList.contains(name)) {
                    commonNames.add(name);
                }
                if (name.equals("")) { // Signal to show that end of file is reached so that the method ends and thread can join
                    break;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

class Client implements Runnable { // Class to represent client connections to the other processes
    private int port; // Port to connect to
    List<String> namesList;

    Client(int port, List<String> namesList) {
        this.port = port;
        this.namesList = namesList;
    }
    @Override
    public void run() {
        boolean connected = false; // Variable to enable repeated attempts to connect to the target server
        while (!connected) {
            try {
                Socket clientSocket = new Socket("localhost", port); // Open client socket to server
                connected = true;

                PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true); // Open output stream to send to server
                for (String name : namesList) { // For each name in the list, send it over the socket
                    out.println(name);
                }
                out.println(""); // When list of names has ended, send empty string to signal end of file

            } catch (IOException e) {
                try {
                    Thread.sleep(1000); // Wait a second before trying to connect again
                } catch (InterruptedException ie) {
                    ie.printStackTrace();
                }
            }
        }
    }
}