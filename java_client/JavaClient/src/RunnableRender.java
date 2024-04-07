import javax.swing.*;

public class RunnableRender implements Runnable{
    static float fps = 0;
    static int frames = 0;
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
            MainGUI.fpsValue.setText("FPS: "+ String.valueOf(fps));
            frames = 0;
        });
        timer.start();
    }
}
