import javax.swing.*;
import java.awt.*;

public class MainGUI {
    public static final JFrame mainWindow = new JFrame();
    static JPanel parentPanel = new JPanel();
    private static final ParticlesDrawArea simulationPanel = new ParticlesDrawArea();

    static JLabel fpsValue = new JLabel("FPS: 0.0");
    static JPanel utilPanel = new JPanel();

    static JLabel userX = new JLabel("User X: " + ParticlesDrawArea.userSprite.x);
    static JLabel userY = new JLabel("User Y: " + ParticlesDrawArea.userSprite.y);


    public static void initGUI() {
        mainWindow.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        parentPanel.setSize(
                (int)Toolkit.getDefaultToolkit().getScreenSize().getWidth(),
                (int)Toolkit.getDefaultToolkit().getScreenSize().getHeight()
        );
        parentPanel.setLayout(new FlowLayout());
        parentPanel.setBackground(Color.GRAY);
        fpsValue.setBackground(Color.WHITE);

        parentPanel.add(simulationPanel);

        simulationPanel.setPreferredSize(
                new Dimension(
                        getScaledWidth(1280),
                        getScaledHeight(720)
                )
        );

        utilPanel.setLayout(new GridLayout(3, 1));
        utilPanel.setPreferredSize(new Dimension(300, 200));
        utilPanel.add(fpsValue);
        utilPanel.add(userX);
        utilPanel.add(userY);

        parentPanel.add(utilPanel);

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
