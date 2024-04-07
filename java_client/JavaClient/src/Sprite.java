public class Sprite {
    float x, y;
    float speedMult;

    Sprite(float x, float y, float speed){
        this.x = x;
        this.y = y;
        this.speedMult = speed;
    }

    void move(float dx, float dy, float frameRate) {
        x += dx * speedMult / frameRate;
        y -= dy * speedMult / frameRate;

        if (x < 0) x = 0;
        if (x > 1280) x = 1280;
        if (y < 0) y = 0;
        if (y > 720) y = 720;

        x = Math.max(0.0f, Math.min(1280.0f, x));
        y = Math.max(0.0f, Math.min(720.0f, y));
    }
}
