library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
	port(
		SMCLK, RESET: in std_logic;
		ROW, LED: out std_logic_vector(0 to 7)
	);
end ledc8x8;

architecture project of ledc8x8 is
	signal clock_enable, timer: std_logic;
	signal row_selector: std_logic_vector(0 to 7);
begin
	-- generuje signal clock_enable s frekvenci SMCLK/256
	ce_siganl: process (SMCLK, RESET)
	variable counter: std_logic_vector(7 downto 0);
	begin
		if (RESET = '1') then
			counter := "00000000";
		elsif (SMCLK'event) and (SMCLK = '1') then
			counter := counter + 1;
			if counter = "11111111" then
				clock_enable <= '1';
			else
				clock_enable <= '0';
			end if;
		end if;
	end process;

	-- pocitadlo odhadujici cas ~0.5s od posledniho resetu (cca 16000nasobek periody clock_enable)
	time_watch: process (SMCLK, RESET, clock_enable)
	variable t_counter: std_logic_vector(13 downto 0);
	begin
		if (RESET = '1') then
			t_counter := "00000000000000";
			timer <= '0';
		elsif (SMCLK'event) and (SMCLK = '1') then
			if (clock_enable = '1') then    
				t_counter := t_counter + 1;
			end if;
			if (t_counter(13) = '1') then
				timer <= '1';
			end if;
		end if;
	end process;

	-- rotacni registr rizeny signalem clock_enable 
	rot_register: process(SMCLK, RESET, clock_enable, row_selector)
	begin
		if (RESET = '1') then
			row_selector <= "10000000"; 
		elsif (SMCLK'event) and (SMCLK = '1') then
			if (clock_enable = '1') then
				row_selector <= row_selector(7) & row_selector(0 to 6);
			end if;
		end if;
	ROW <= row_selector;    
	end process;

	-- dekoder inicial
	decoder: process(SMCLK, RESET, timer, row_selector)
	begin
		
		if (RESET = '1') then
			LED <= "11111111";			
		elsif (SMCLK'event) and (SMCLK = '1') then
			if (timer = '1') then 
				case row_selector is
					when "10000000" => LED <= "10011111";
					when "01000000" => LED <= "01101111";
					when "00100000" => LED <= "01101111";
					when "00010000" => LED <= "01101111";
					when "00001000" => LED <= "10001110";
					when "00000100" => LED <= "11110101";
					when "00000010" => LED <= "11110101";
					when "00000001" => LED <= "11111011";
					when others => LED <= "11111111"; 
				end case;
			else 
				case row_selector is
					when "10000000" => LED <= "10011111";
					when "01000000" => LED <= "01101111";
					when "00100000" => LED <= "01101111";
					when "00010000" => LED <= "01101111";
					when "00001000" => LED <= "10011111";
					when "00000100" => LED <= "11111111";
					when "00000010" => LED <= "11111111";
					when "00000001" => LED <= "11111111";
					when others => LED <= "11111111"; 
				end case; 
			end if;
		end if;	
	end process;
end project;

