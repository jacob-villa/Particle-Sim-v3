import javax.swing.*;
import java.awt.event.ActionEvent;



public class Controller {


    public static void initListeners(){

        MainGUI.parentPanel.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke("pressed W"),
                "pressedW"
        );
        MainGUI.parentPanel.getActionMap().put("pressedW", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Main.tasksQueue.add(new Task(ParticlesDrawArea.userSprite, 0, -ParticlesDrawArea.spriteSpeed, ParticlesDrawArea.frameRate));
                MainGUI.userY.setText("User Y: " + ParticlesDrawArea.userSprite.y);
            }
        });


        MainGUI.parentPanel.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke("pressed A"),
                "pressedA"
        );
        MainGUI.parentPanel.getActionMap().put("pressedA", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Main.tasksQueue.add(new Task(ParticlesDrawArea.userSprite, -ParticlesDrawArea.spriteSpeed, 0, ParticlesDrawArea.frameRate));
                MainGUI.userX.setText("User X: " + ParticlesDrawArea.userSprite.x);
            }
        });


        MainGUI.parentPanel.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke("pressed S"),
                "pressedS"
        );
        MainGUI.parentPanel.getActionMap().put("pressedS", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Main.tasksQueue.add(new Task(ParticlesDrawArea.userSprite, 0, ParticlesDrawArea.spriteSpeed, ParticlesDrawArea.frameRate));
                MainGUI.userY.setText("User Y: " + ParticlesDrawArea.userSprite.y);
            }
        });


        MainGUI.parentPanel.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke("pressed D"),
                "pressedD"
        );
        MainGUI.parentPanel.getActionMap().put("pressedD", new AbstractAction() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Main.tasksQueue.add(new Task(ParticlesDrawArea.userSprite, ParticlesDrawArea.spriteSpeed, 0, ParticlesDrawArea.frameRate));
                MainGUI.userX.setText("User X: " + ParticlesDrawArea.userSprite.x);
            }
        });

    }
}
