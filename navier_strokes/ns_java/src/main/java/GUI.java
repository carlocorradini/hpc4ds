import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.*;

public class GUI extends JPanel implements ActionListener, ItemListener {
    private static final long serialVersionUID = 1L;
    JCheckBox fluid = new JCheckBox("Show Fluid"), vector = new JCheckBox("Show Vector Field"), right = new JCheckBox("Right Click To Apply Force");
    JLabel visco = new JLabel("Viscocity:"), diffu = new JLabel("Diffusion:"), times = new JLabel("Time Step:"), gasde = new JLabel("Density:");
    JButton setV = new JButton("Set"), setD = new JButton("Set"), setDT = new JButton("Set"), setG = new JButton("Set"), clear = new JButton("CLEAR"), changeColor = new JButton("Gas Colour"), backgroundColor = new JButton("BG Colour");
    JTextField viscocity = new JTextField("0.0001"), diffusionRate = new JTextField("0.0001"), timeStep = new JTextField("0.01"), gasDens = new JTextField("10");
    ButtonGroup radioMenu = new ButtonGroup();
    JRadioButton normal = new JRadioButton("Colour"), rainbow = new JRadioButton("Rainbow"), fire = new JRadioButton("Fire");
    JButton tick = new JButton("TICK");

    public GUI(int x, int y, int width, int height) {
        this.setBounds(x, y, width, height);
        this.setLayout(null);
        radioMenu.add(normal);
        normal.setSelected(true);
        radioMenu.add(rainbow);
        radioMenu.add(fire);
        normal.setBounds(5, 185, 70, 20);
        rainbow.setBounds(80, 185, 80, 20);
        fire.setBounds(165, 185, 80, 20);

        fluid.setBounds(5, 5, 90, 20);
        vector.setBounds(100, 5, 130, 20);
        right.setBounds(5, 35, 200, 20);
        clear.setBounds(295, 5, 80, 20);
        changeColor.setBounds(255, 35, 120, 20);
        backgroundColor.setBounds(255, 65, 120, 20);
        tick.setBounds(255, 95, 120, 20);

        visco.setBounds(5, 65, 60, 20);
        viscocity.setBounds(70, 65, 100, 20);
        setV.setBounds(175, 65, 53, 20);

        diffu.setBounds(5, 95, 60, 20);
        diffusionRate.setBounds(70, 95, 100, 20);
        setD.setBounds(175, 95, 53, 20);

        times.setBounds(5, 125, 60, 20);
        timeStep.setBounds(70, 125, 100, 20);
        setDT.setBounds(175, 125, 53, 20);

        gasde.setBounds(5, 155, 60, 20);
        gasDens.setBounds(70, 155, 100, 20);
        setG.setBounds(175, 155, 53, 20);

        fluid.setSelected(true);
        vector.setSelected(false);
        right.setSelected(true);
        this.add(fluid);
        this.add(vector);
        this.add(right);
        this.add(visco);
        this.add(viscocity);
        this.add(setV);
        this.add(diffu);
        this.add(diffusionRate);
        this.add(setD);
        this.add(times);
        this.add(timeStep);
        this.add(setDT);
        this.add(gasde);
        this.add(gasDens);
        this.add(setG);
        this.add(changeColor);
        this.add(backgroundColor);
        this.add(tick);
        this.add(clear);
        this.add(normal);
        this.add(rainbow);
        this.add(fire);


        fluid.addItemListener(this);
        vector.addItemListener(this);
        right.addItemListener(this);
        normal.addItemListener(this);
        rainbow.addItemListener(this);
        fire.addItemListener(this);
        setV.addActionListener(this);
        setD.addActionListener(this);
        setDT.addActionListener(this);
        setG.addActionListener(this);
        clear.addActionListener(this);
        changeColor.addActionListener(this);
        backgroundColor.addActionListener(this);
        tick.addActionListener(this);
		
		this.C_testing();
    }

    public void itemStateChanged(ItemEvent e) {
        if (e.getItemSelectable() == normal) {
            Render.normalMode = true;
            Render.fireMode = false;
            Render.rainbowMode = false;
        }
        if (e.getItemSelectable() == rainbow) {
            Render.normalMode = false;
            Render.fireMode = false;
            Render.rainbowMode = true;
        }
        if (e.getItemSelectable() == fire) {
            Render.normalMode = false;
            Render.fireMode = true;
            Render.rainbowMode = false;
        }

        if (e.getItemSelectable() == fluid) {
            if (fluid.isSelected()) {
                fluid.setSelected(true);
                Render.drawFluid = true;
            } else {
                fluid.setSelected(false);
                Render.drawFluid = false;
            }
        }
        if (e.getItemSelectable() == vector) {
            if (vector.isSelected()) {
                vector.setSelected(true);
                Render.drawVectorField = true;
            } else {
                vector.setSelected(false);
                Render.drawVectorField = false;
            }
        }
        if (e.getItemSelectable() == right) {
            if (right.isSelected()) {
				System.out.println("I'M ON TRUE");
                right.setSelected(true);
                Render.rightEnabled = true;
            } else {
				System.out.println("I'M ON FALSE");
                right.setSelected(false);
                Render.rightEnabled = false;
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        if (e.getSource() == setV) {
            try {
                Render.visc = Double.parseDouble(viscocity.getText());
            } catch (Exception exc) {
                System.out.println("Invalid parameter");
                viscocity.setText(Double.toString(Render.visc));
            }
        }
        if (e.getSource() == setD) {
            try {
                Render.diff = Double.parseDouble(diffusionRate.getText());
            } catch (Exception exc) {
                System.out.println("Invalid parameter");
                diffusionRate.setText(Double.toString(Render.diff));
            }
        }
        if (e.getSource() == setDT) {
            try {
                Render.dt = Double.parseDouble(timeStep.getText());
            } catch (Exception exc) {
                System.out.println("Invalid parameter");
                timeStep.setText(Double.toString(Render.dt));
            }
        }
        if (e.getSource() == setG) {
            try {
                Render.gasDensity = Double.parseDouble(gasDens.getText());
            } catch (Exception exc) {
                System.out.println("Invalid parameter");
                gasDens.setText(Double.toString(Render.gasDensity));
            }
        }
        if (e.getSource() == clear) {
            Render.solver.clear();
        }
        if (e.getSource() == changeColor) {
            Color tempCol = new Color(0, 0, 0);
            try {
                tempCol = JColorChooser.showDialog(this, "Pick a Color", Render.col);
            } catch (Exception exc) {
                System.out.println("Invalid parameter");
                tempCol = null;
            }
            if (tempCol != null) {
                Render.col = tempCol;
            }
        }
        if (e.getSource() == backgroundColor) {
            Color tempCol = new Color(0, 0, 0);
            try {
                tempCol = JColorChooser.showDialog(this, "Pick a Color", Render.bgCol);
            } catch (Exception exc) {
                System.out.println("Invalid parameter");
                tempCol = null;
            }
            if (tempCol != null) {
                Render.bgCol = tempCol;
            }
        }

        if(e.getSource() == tick) {
            Render.solver.tick(Render.dt, Render.visc, Render.diff);
        }
    }
	
	/**
	 * THIS IS A SEQUENTIAL IMPLEMENTATION OF THE SOLVER
	 * We'll use this function to reproduce the same steps of the C 
	 * counterpart, that will be sequential/parallel w.r.t. this one.
	 */
	public void C_testing() {
		double FLUID_VISCOSITY = 0.0001;
		double FLUID_DIFFUSION = 0.0001;
		double NS_TIME_STEP = 0.01;
		int N_TICKS = 50;
		
		this.ns_increase_density(41, 41);
		this.ns_increase_density(65, 20);
		this.ns_increase_density(15, 20);
		this.ns_apply_force(41, 41, 0, 80);
		
		for (int i=0; i<N_TICKS +1; i++) {
			Render.solver.tick(0.01, 0.0001, 0.0001);
		}
	}
	
	/**
	 * Wrapper useful to us
	 * @param x
	 * @param y 
	 */
	static void ns_increase_density(int x, int y) {
		int INCREASE = 10;
		Render.solver.dense[Render.solver.INDEX(x, y)] += INCREASE;
	}
	
	/**
	 * Other useful wrapper
	 * @param x
	 * @param y
	 * @param v_x
	 * @param v_y 
	 */
	static void ns_apply_force(int x, int y, int v_x, int v_y) {
		Render.solver.applyForce(41, 41, v_x, v_y);
	}
}
