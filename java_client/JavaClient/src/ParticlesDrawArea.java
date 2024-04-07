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
    public static int ovalSize = 10;
    public static int particleSize = 3;

    ParticlesDrawArea() {
        super();
        this.setBackground(Color.BLACK);
        this.setOpaque(true);

        particles.add(new Particle(10, 90, 35, 100));
        particles.add(new Particle(10, 150, 35, 100));
        particles.add(new Particle(10, 200, 35, 100));
    }

    public void paintComponent(Graphics g){
        super.paintComponent(g);

        g2d = (Graphics2D) g.create();

//        g2d.translate(
//                MainGUI.getScaledWidth((int)userSprite.x),
//                MainGUI.getScaledHeight((int)userSprite.y)
//        );

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
                MainGUI.getScaledWidth((int)userSprite.x) - (ovalSize/2),
                MainGUI.getScaledHeight(720) - MainGUI.getScaledHeight((int)userSprite.y) - (ovalSize/2),
                ovalSize,
                ovalSize
        );
//        System.out.println(particles.size());

        for(int i = 0; i < particles.size(); i++) {
            Particle p = ParticlesDrawArea.particles.get(i);
//            if(p.x >= sprite.x - 9 && p.x <= (sprite.x + 288) && p.y >= sprite.y - 9 && p.y <= (sprite.y + 162)){
//                g.setColor(Color.white);
//                g.fillOval(MainGUI.getScaledWidth((int)p.x), MainGUI.getScaledHeight((int)p.y), ParticlesDrawArea.ovalSize, ParticlesDrawArea.ovalSize);
//            } else {
//                g.setColor(Color.white);
//                g.fillOval(MainGUI.getScaledWidth((int)p.x), MainGUI.getScaledHeight((int)p.y), ParticlesDrawArea.ovalSize, ParticlesDrawArea.ovalSize);
//            }
            g.setColor(Color.white);
            g.fillOval(MainGUI.getScaledWidth((int)p.x) - (particleSize/2), MainGUI.getScaledHeight((int)p.y) - (particleSize/2), ParticlesDrawArea.particleSize, ParticlesDrawArea.particleSize);
            // arbitrary framerate when initialized
            p.updatePosition(RunnableRender.fps > 350 || RunnableRender.fps <= 0  ? 250 : RunnableRender.fps);
        }

        RunnableRender.frames++;

        g2d.dispose();
        repaint();
    }
}