import javax.swing.*;
import java.util.concurrent.atomic.AtomicInteger;

public class RunnableRender implements Runnable{
    static float fps = 300;
    static int frames = 300;
    @Override
    public void run() {
        runTimers();
        Timer timer = new Timer((0), e -> {
            SwingUtilities.invokeLater(() -> {
                MainGUI.parentPanel.repaint();
            });
        });

        timer.start();
    }

    private void runTimers() {
        Timer timer = new Timer(500, e ->{
            fps = frames / 0.5f;
            MainGUI.fpsValue.setText("FPS: "+ fps);
            frames = 0;
        });
        timer.start();
    }
}
