import javax.swing.*;
import java.awt.*;
import java.awt.geom.AffineTransform;
import java.util.concurrent.CopyOnWriteArrayList;

public class ParticlesDrawArea extends JPanel {
    public static float frameRate = 60;

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

        particles.add(new Particle(10, 90, 35, 300));
        particles.add(new Particle(10, 150, 35, 300));
        particles.add(new Particle(10, 200, 35, 300));
        particles.add(new Particle(10, 300, 35, 300));
        particles.add(new Particle(10, 400, 35, 300));
    }

    public void paintComponent(Graphics g){
        super.paintComponent(g);

        g2d = (Graphics2D) g.create();

        double zoomX = (double) getWidth() / MainGUI.getScaledWidth(33);
        double zoomY = (double) getHeight() / MainGUI.getScaledHeight(19);


        AffineTransform at = new AffineTransform();
        at.translate(getWidth() / 2f, getHeight() / 2f);
        at.scale(zoomX, zoomY);
        at.translate(
                -MainGUI.getScaledWidth((int) userSprite.x)+(ovalSize/2f),
                MainGUI.getScaledHeight((int) userSprite.y) - MainGUI.getScaledHeight(720)
        );
        g2d.setTransform(at);

        g2d.setColor(Color.WHITE);
        g2d.fillRect(MainGUI.getScaledWidth(-144), MainGUI.getScaledHeight(-81), MainGUI.getScaledWidth(144), MainGUI.getScaledHeight(850));
        g2d.fillRect(MainGUI.getScaledWidth(1280), MainGUI.getScaledHeight(-81), MainGUI.getScaledWidth(144), MainGUI.getScaledHeight(850));
        g2d.fillRect(MainGUI.getScaledWidth(-144), MainGUI.getScaledHeight(-81), MainGUI.getScaledWidth(1280 + 144), MainGUI.getScaledHeight(81));
        g2d.fillRect(MainGUI.getScaledWidth(-144), MainGUI.getScaledHeight(720), MainGUI.getScaledWidth(1280 + 144), MainGUI.getScaledHeight(81));

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
            if(p.x >= userSprite.x - 9 && p.x <= (userSprite.x + 288) && p.y >= userSprite.y - 9 && p.y <= (userSprite.y + 162)){
                g2d.setColor(Color.white);
                g2d.fillOval(
                        MainGUI.getScaledWidth((int)p.x) - (particleSize/2),
                        MainGUI.getScaledHeight(720) - MainGUI.getScaledHeight((int)p.y) - (particleSize/2),
                        particleSize,
                        particleSize
                );
            }
            g2d.setColor(Color.white);
            g2d.fillOval(
                    MainGUI.getScaledWidth((int)p.x) - (particleSize/2),
                    MainGUI.getScaledHeight(720) - MainGUI.getScaledHeight((int)p.y) - (particleSize/2),
                    particleSize,
                    particleSize
            );
            // arbitrary framerate when initialized
            p.updatePosition(RunnableTimer.fps > 350 || RunnableTimer.fps <= 0  ? 250 : RunnableTimer.fps);
        }

        RunnableTimer.frames++;

        g2d.dispose();
        repaint();
    }
}