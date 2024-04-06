import javax.swing.*;
import java.awt.*;

public class ParticlesDrawArea extends JPanel {
    static final float scaleFactorWidth = 1280.0f / 19.0f;
    static final float scaleFactorHeight = 720.0f / 33.0f;
    static final float spriteStartX = 640f;
    static final float spriteStartY = 360f;

    Graphics2D g2d = null;

    ParticlesDrawArea() {
        super();
        this.setBackground(Color.BLACK);
    }
}