import javax.swing.*;
import java.awt.*;
import java.awt.geom.AffineTransform;
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
    private int ovalSize = 10;

    ParticlesDrawArea() {
        super();
        this.setBackground(Color.BLACK);
        this.setOpaque(true);
    }

    public void paintComponent(Graphics g){
        super.paintComponent(g);

        g2d = (Graphics2D) g.create();

//        g2d.translate(
//                MainGUI.getScaledWidth((int)userSprite.x),
//                MainGUI.getScaledHeight((int)userSprite.y)
//        );
//
//        g2d.setTransform(new AffineTransform());
//        g2d.scale(
//                (double) getWidth() / MainGUI.getScaledWidth(33 * ovalSize),
//                (double) getHeight() / MainGUI.getScaledHeight(19 * ovalSize)
//        );
//
//        g2d.setColor(Color.WHITE);
//        g2d.fillRect(MainGUI.getScaledWidth(-144), MainGUI.getScaledHeight(-81), MainGUI.getScaledWidth(144), MainGUI.getScaledHeight(850));
//        g2d.fillRect(MainGUI.getScaledWidth(1280), MainGUI.getScaledHeight(-81), MainGUI.getScaledWidth(144), MainGUI.getScaledHeight(850));
//        g2d.fillRect(MainGUI.getScaledWidth(-144), MainGUI.getScaledHeight(-81), MainGUI.getScaledWidth(1280 + 144), MainGUI.getScaledHeight(81));
//        g2d.fillRect(MainGUI.getScaledWidth(-144), MainGUI.getScaledHeight(720), MainGUI.getScaledWidth(1280 + 144), MainGUI.getScaledHeight(81));
//
//
//
//        g2d.fillRect(MainGUI.getScaledWidth(-9999), MainGUI.getScaledHeight(-9999), MainGUI.getScaledWidth(9999), MainGUI.getScaledHeight(9999));

        g2d.setColor(Color.BLUE);
        g2d.fillOval(
                MainGUI.getScaledWidth((int)userSprite.x),
                MainGUI.getScaledHeight(720) - MainGUI.getScaledHeight((int)userSprite.y),
                ovalSize,
                ovalSize
        );
    }


}