package WelfareCrook;

import java.io.*;
import java.net.*;
import java.util.*;

public class ProcessG {
    public static void main(String[] args) {
        String fileName = "process_G_data.txt";
        List<String> namesList = new ArrayList<>();

        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {
            String line;
            while ((line = br.readLine()) != null) {
                namesList.add(line);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        new Thread(new ClientG(5001, namesList)).start();
        new Thread(new ClientG(5003, namesList)).start();

        int numberOfConnections = 0;
        List<String> commonNamesFirstConnection = new ArrayList<>();
        List<String> commonNamesSecondConnection = new ArrayList<>();
        List<Thread> serverThreads = new ArrayList<>();

        try (ServerSocket welcomeSocket = new ServerSocket(5002)) {
            System.out.println("Process G listening for incoming connections...");

            for (int i = 0; i < 2; i++) {
                Socket connectionSocket = welcomeSocket.accept();
                System.out.println("Process G received connection");

                ServerG server;
                if (i == 0) {
                    server = new ServerG(namesList, connectionSocket, commonNamesFirstConnection);
                } else {
                    server = new ServerG(namesList, connectionSocket, commonNamesSecondConnection);
                }

                Thread serverThread = new Thread(server);
                serverThread.start();
                serverThreads.add(serverThread);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        for (Thread thread : serverThreads) {
            try {
                thread.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        commonNamesFirstConnection.retainAll(commonNamesSecondConnection);

        System.out.println("Process G result:");
        System.out.println(commonNamesFirstConnection);
    }
}

class ServerG implements Runnable {
    List<String> namesList;
    Socket connectionSocket;
    private List<String> commonNames;

    ServerG(List<String> namesList, Socket connectionSocket, List<String> commonNames) {
        this.namesList = namesList;
        this.connectionSocket = connectionSocket;
        this.commonNames = commonNames;
    }

    @Override
    public void run() {
        try {
            BufferedReader in = new BufferedReader(new InputStreamReader(connectionSocket.getInputStream()));
            String name;
            while ((name = in.readLine()) != null) {
                if (namesList.contains(name)) {
                    commonNames.add(name);
                }
                if (name.equals("")) {
                    break;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

class ClientG implements Runnable {
    private int port;
    List<String> namesList;

    ClientG(int port, List<String> namesList) {
        this.port = port;
        this.namesList = namesList;
    }
    @Override
    public void run() {
        boolean connected = false;
        while (!connected) {
            try {
                Socket clientSocket = new Socket("localhost", port);
                connected = true;

                PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);
                for (String name : namesList) {
                    out.println(name);
                }
                out.println("");

            } catch (IOException e) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException ie) {
                    ie.printStackTrace();
                }
            }
        }
    }
}