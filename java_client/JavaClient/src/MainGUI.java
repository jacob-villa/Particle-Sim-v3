import javax.swing.*;
import java.awt.*;

public class MainGUI {
    public static final JFrame mainWindow = new JFrame();
    static JPanel parentPanel = new JPanel();
    private static final ParticlesDrawArea simulationPanel = new ParticlesDrawArea();

    public static void initializeGUI() {
        mainWindow.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        parentPanel.setSize(
                (int)Toolkit.getDefaultToolkit().getScreenSize().getWidth(),
                (int)Toolkit.getDefaultToolkit().getScreenSize().getHeight()
        );
        parentPanel.setBackground(Color.GRAY);
        parentPanel.add(simulationPanel);

        simulationPanel.setPreferredSize(
                new Dimension(
                        getScaledWidth(1280),
                        getScaledHeight(720)
                )
        );

        // Add panel to the center of the content pane
        mainWindow.getContentPane().add(parentPanel, BorderLayout.CENTER);
        mainWindow.setSize(
                (int)Toolkit.getDefaultToolkit().getScreenSize().getWidth(),
                (int)Toolkit.getDefaultToolkit().getScreenSize().getHeight()
        );
        mainWindow.setExtendedState(JFrame.MAXIMIZED_BOTH);

        mainWindow.setVisible(true);
    }

    public static int getScaledWidth(int width){
        int absoluteScreenWidth = mainWindow.getGraphicsConfiguration().getDevice().getDisplayMode().getWidth();
        int scaledScreenWidth = (int)Toolkit.getDefaultToolkit().getScreenSize().getWidth();

        return width * scaledScreenWidth / absoluteScreenWidth;
    }

    public static int getScaledHeight(int height){
        int absoluteScreenHeight = mainWindow.getGraphicsConfiguration().getDevice().getDisplayMode().getHeight();
        int scaledScreenHeight = (int)Toolkit.getDefaultToolkit().getScreenSize().getHeight();

        return height * scaledScreenHeight / absoluteScreenHeight;
    }
}
