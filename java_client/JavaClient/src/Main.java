import java.util.concurrent.DelayQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Main {

    static final int THREAD_COUNT = Runtime.getRuntime().availableProcessors();
    public static ExecutorService threadPool = Executors.newFixedThreadPool(THREAD_COUNT);
    public static DelayQueue<Task> tasksQueue = new DelayQueue<>();


    public static void main(String[] args) throws InterruptedException {
        Controller.initListeners();
        MainGUI.initGUI();
        RunnableRender renderTask = new RunnableRender();
        Thread renderThread = new Thread(renderTask);

        threadPool.execute(new RunnableTask(tasksQueue.take()) {
        });
        renderThread.start();
        while(true) {
            threadPool.execute(new RunnableTask(tasksQueue.take()));
        }
    }
}