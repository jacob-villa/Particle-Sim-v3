public class Particle {
    float x, y;
    float angle;
    float velocity;
    static final float PI = 3.14159265359F;


    Particle(float x, float y, float angle, float velocity) {
        this.x = x;
        this.y = y;
        this.angle = angle;
        this.velocity = velocity;
    }

    public void updatePosition(float frameRate){
        float radians = (float) ((angle * PI) / 180.0);

        float dx = (float) (Math.cos(radians) * velocity / frameRate);
        float dy = (float) (Math.sin(radians) * velocity / frameRate);

        float newX = x + dx;
        float newY = y + dy;

        // Threshold for collision detection
        float threshold = velocity > 500 ? 10.0f : 3.0f;

        boolean collisionDetected = false;

        x = newX;
        y = newY;

        if (x < 0) {
            x = 0;
            angle = 180 - angle;
        }
        else if (x > 1280) {
            x = 1280;
            angle = 180 - angle;
        }

        if (y < 0) {
            y = 0;
            angle = -angle;
        }
        else if (y > 720) {
            y = 720;
            angle = -angle;
        }

        // Ensure particles stay within the 1280x720 black panel
        x = Math.max(0.0f, Math.min(1280.0f, x));
        y = Math.max(0.0f, Math.min(720.0f, y));
    }
}
