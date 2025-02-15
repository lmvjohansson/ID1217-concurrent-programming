import java.time.LocalTime;
import java.time.format.DateTimeFormatter;

public class UnisexBathroom {
    public static void main(String[] args) {
        Bathroom bathroom = new Bathroom();

        for (int i = 0; i < 5; i++) {
            Woman woman = new Woman(i, bathroom);
            Thread womanThread = new Thread(woman);
            womanThread.start();
        }

        for (int i = 0; i < 5; i++) {
            Man man = new Man(i, bathroom);
            Thread manThread = new Thread(man);
            manThread.start();
        }
    }
}

class Bathroom {
    DateTimeFormatter formatter = DateTimeFormatter.ofPattern("HH:mm:ss.SS"); // To limit the number of decimals printed by LocalTime.now()
    private int womenCount = 0;
    private int manCount = 0;
    private int waitingWomen = 0;
    private int waitingMen = 0;
    private char occupiedBy = 'N'; // Set to M when occupied by men, set to W when occupied by women, set to N when empty
    private char nextTurn = 'N'; // Set to M when women are inside the bathroom and there are men waiting and vice versa

    public synchronized void womanEnter(int id) {
        waitingWomen++;
        while (occupiedBy == 'M' || (nextTurn == 'M' && waitingMen > 0)) { // Women wait if bathroom is occupied by men or men are ahead in queue
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        waitingWomen--;

        if (womenCount == 0) { // First woman signals bathroom as occupied by women
            occupiedBy = 'W';
        }
        womenCount++;
        System.out.println(LocalTime.now().format(formatter) + " - Woman " + id + " entered bathroom, number of women in the bathroom is " + womenCount); // + " number of waiting men " + waitingMen);
    }

    public synchronized void womanExit(int id) {
        womenCount--;
        System.out.println(LocalTime.now().format(formatter) + " - Woman " + id + " left bathroom, number of women in the bathroom is " + womenCount); // + " number of waiting men " + waitingMen);
        if (waitingMen > 0) { // Every time a woman exits the queue is checked for men, if there are men waiting they should have the next turn to use bathroom
            nextTurn = 'M';
        } else { // If there is no one waiting the bathroom is open to anyone
            nextTurn = 'N';
        }
        if (womenCount == 0) { // Last woman to leave signals that the bathroom is empty and notifies all threads
            occupiedBy = 'N';
            notifyAll();
        }
    }

    public synchronized void manEnter(int id) {
        waitingMen++;
        while (occupiedBy == 'W' || (nextTurn == 'W' && waitingWomen > 0)) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        waitingMen--;

        if (manCount == 0) {
            occupiedBy = 'M';
        }
        manCount++;
        System.out.println(LocalTime.now().format(formatter) + " - Man " + id + " entered bathroom, number of men in the bathroom is " + manCount); // + " number of waiting women " + waitingWomen);
    }

    public synchronized void manExit(int id) {
        manCount--;
        System.out.println(LocalTime.now().format(formatter) + " - Man " + id + " left bathroom, number of men in the bathroom is " + manCount); // + " number of waiting women " + waitingWomen);
        if (waitingWomen > 0) {
            nextTurn = 'W';
        } else {
            nextTurn = 'N';
        }
        if (manCount == 0) {
            occupiedBy = 'N';
            notifyAll();
        }
    }
}

class Woman implements Runnable {
    int id;
    Bathroom bathroom;

    Woman(int id, Bathroom b) {
        this.id = id;
        bathroom = b;
    }

    @Override
    public void run() {
        while (true) {
            bathroom.womanEnter(id);

            try {
                Thread.sleep(1000 + (int)(Math.random() * 3000));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            bathroom.womanExit(id);

            try {
                Thread.sleep(5000 + (int)(Math.random() * 3000));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}

class Man implements Runnable {
    int id;
    Bathroom bathroom;

    Man(int id, Bathroom b) {
        this.id = id;
        bathroom = b;
    }

    @Override
    public void run() {
        while (true) {
            bathroom.manEnter(id);

            try {
                Thread.sleep(1000 + (int)(Math.random() * 3000)); // Random wait between 1 and 4 seconds
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            bathroom.manExit(id);

            try {
                Thread.sleep(5000 + (int)(Math.random() * 3000)); // Random wait between 1 and 4 seconds
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}