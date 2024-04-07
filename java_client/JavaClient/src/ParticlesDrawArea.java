import javax.swing.*;
import java.awt.*;
import java.util.concurrent.CopyOnWriteArrayList;

public class ParticlesDrawArea extends JPanel {
    public static float frameRate = 60;
    static final float scaleFactorWidth = 1280.0f / 19.0f;
    static final float scaleFactorHeight = 720.0f / 33.0f;

    static final float spriteStartX = 640f;
    static final float spriteStartY = 360f;
    private static final float spriteSpeedMult = 10f;
    public static final float spriteSpeed = 30f;
    public static final Sprite userSprite = new Sprite(
            spriteStartX,
            spriteStartY,
            spriteSpeedMult
    );

    static CopyOnWriteArrayList<Particle> particles = new CopyOnWriteArrayList<>();

    public Graphics2D g2d = null;

    ParticlesDrawArea() {
        super();
        this.setBackground(Color.BLACK);
        this.setOpaque(true);
    }

    public void paintComponent(Graphics g){
        super.paintComponent(g);

        g2d = (Graphics2D) g.create();
    }


}