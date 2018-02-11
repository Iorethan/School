-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2015 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): DOPLNIT
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

	signal pc_reg : std_logic_vector(12 downto 0);
	signal pc_inc : std_logic;
	signal pc_dec : std_logic;

	signal ptr_reg : std_logic_vector(12 downto 0);
	signal ptr_inc : std_logic;
	signal ptr_dec : std_logic;

	signal cnt_reg : std_logic_vector(7 downto 0);
	signal cnt_inc : std_logic;
	signal cnt_dec : std_logic;
	
	signal tmp_reg : std_logic_vector(7 downto 0);
	signal tmp_ld : std_logic;
	
	signal data_addr_sel : std_logic;	-- 0 for PC
										-- 1 for PTR
	
	signal data_wdata_sel : std_logic_vector(1 downto 0);	-- 00 for IN_DATA
															-- 01 for TMP
															-- 10 for DATA_RDATA - 1
															-- 11 for DATA_RDATA + 1
	
	type fsm_state is (sidle, sfetch, sfetch1, sdecode, sincp, sdecp, sinc, sinc1, sdec,
					   sdec1, swhile, snwhile, sprint, sprint1, sread, sread1, sstore,
					   sstore1, sload, snop, shalt, swhile1, swhile2, swhile3, snwhile1,
					   snwhile2, snwhile3, snwhile4);
	signal pstate : fsm_state;
	signal nstate : fsm_state;

begin

	-- registr programoveho citace
	pc_proc: process (CLK, RESET, pc_inc, pc_dec, pc_reg)
	begin
		if (RESET = '1') then
			pc_reg <= (others => '0');
		elsif (CLK'event) and (CLK = '1') then
			if (pc_inc = '1') then
				pc_reg <= pc_reg + 1;
			end if;
			if (pc_dec = '1') then
				pc_reg <= pc_reg - 1;
			end if;
		end if;
	end process;
	
	-- registr ukazatele do pameti (kruhovy, hodnoty od X"1000" do X"1FFF")
	ptr_proc: process (CLK, RESET, ptr_inc, ptr_dec, ptr_reg)
	begin
		if (RESET = '1') then
			ptr_reg <= "1000000000000";
		elsif (CLK'event) and (CLK = '1') then
			if (ptr_inc = '1') then
				if (ptr_reg = "1111111111111") then
					ptr_reg <= "1000000000000";
				else
					ptr_reg <= ptr_reg + 1;
				end if;
			elsif (ptr_dec = '1') then
				if (ptr_reg = "1000000000000") then
					ptr_reg <= "1111111111111";
				else
					ptr_reg <= ptr_reg - 1;
				end if;
			end if;
		end if;
	end process;

	-- registr citace vnorenych cyklu
	cnt_proc: process (CLK, RESET, cnt_inc, cnt_dec, cnt_reg)
	begin
		if (RESET = '1') then
			cnt_reg <= (others => '0');
		elsif (CLK'event) and (CLK = '1') then
			if (cnt_inc = '1') then
			cnt_reg <= cnt_reg + 1;
			elsif (cnt_dec = '1') then
				cnt_reg <= cnt_reg - 1;
			end if;
		end if;
	end process;

	-- registr pro ukladani hodnoty bunky
	tmp_proc: process (CLK, tmp_ld, DATA_RDATA)
	begin
		if (CLK'event) and (CLK = '1') then
			if (tmp_ld = '1') then
				tmp_reg <= DATA_RDATA;
			end if;
		end if;
	end process;
	
	-- multiplexor pro vyber ukazatele do pameti (pc/ptr)
	data_addr_mux: process (data_addr_sel, pc_reg, ptr_reg)
	begin
		if (data_addr_sel = '0') then
			DATA_ADDR <= pc_reg;
		else
			DATA_ADDR <= ptr_reg;
		end if;
	end process;
	
	-- multiplexor pro vyber zapisovanych dat
	data_wdata_mux: process (data_wdata_sel, IN_DATA, tmp_reg, DATA_RDATA)
	begin
		case data_wdata_sel is
			when "00" => DATA_WDATA <= IN_DATA;
			when "01" => DATA_WDATA <= tmp_reg;
			when "10" => DATA_WDATA <= DATA_RDATA - 1;
			when others => DATA_WDATA <= DATA_RDATA + 1;
		end case;
	end process;
	
	-- rizeni prechodu stavu fsm
	fsm_pstate: process (CLK, EN, RESET, nstate)
	begin
		if (RESET = '1') then
			pstate <= sidle;
		elsif (CLK'event) and (CLK = '1') then
			if (EN = '1') then
				pstate <= nstate;
			end if;
		end if;
	end process;
	
	-- rizeni vyberu nasledujiciho stavu
	fsm_nsl: process (EN, pstate, DATA_RDATA, OUT_BUSY, IN_VLD, cnt_reg)
	begin
		-- inicializace
		DATA_EN <= '0';
		DATA_RDWR <= '0';
		if (pstate = sread) then	-- vyresilo warning incomplete FF/latch
			IN_REQ <= '1';
		else
			IN_REQ <= '0';
		end if;
		OUT_WE <= '0';
		pc_inc <= '0';
		pc_dec <= '0';
		ptr_inc <= '0';
		ptr_dec <= '0';
		cnt_inc <= '0';
		cnt_dec <= '0';
		tmp_ld <= '0';
		if (pstate = sinc1) then	-- vyresilo warning incomplete FF/latch			
			data_wdata_sel <= "11";
		elsif (pstate = sdec1) then
			data_wdata_sel <= "10";
		elsif (pstate = sload) then
			data_wdata_sel <= "01";
		else
			data_wdata_sel <= (others => '0');
		end if;
		if (pstate = sinc) or (pstate = sinc1) or (pstate = sdec) or (pstate = sdec1) or (pstate = sprint)	-- vyresilo warning incomplete FF/latch
			or (pstate = sprint1) or (pstate = sread) or (pstate = sread1)  or (pstate = sstore)
			or (pstate = sstore1) or (pstate = sload) or (pstate = swhile) or (pstate = snwhile) then
			data_addr_sel <= '1';
		else
			data_addr_sel <= '0';
		end if;
		OUT_DATA <= DATA_RDATA;
		if (EN = '1') then
			case pstate is
				when sidle =>
					nstate <= sfetch;
				-- nacteni instrukce
				when sfetch =>
					nstate <= sfetch1;
					data_addr_sel <= '0';
					DATA_RDWR <= '1';
					DATA_EN <= '1';
				when sfetch1 =>				
					nstate <= sdecode;
				-- dekodovani instrukce
				when sdecode =>
					DATA_EN <= '0';
					case DATA_RDATA is
						when X"3E" =>
							nstate <= sincp;
						when X"3C" =>
							nstate <= sdecp;
						when X"2B" =>
							nstate <= sinc;
						when X"2D" =>
							nstate <= sdec;
						when X"5B" =>
							nstate <= swhile;
						when X"5D" =>
							nstate <= snwhile;
						when X"2E" =>
							nstate <= sprint;
						when X"2C" =>
							nstate <= sread;
						when X"24" =>
							nstate <= sstore;
						when X"21" =>
							nstate <= sload;
						when X"00" =>
							nstate <= shalt;
						when others =>
							nstate <= snop;					
					end case;
				-- instrukce posunu ukazatele na bunku
				when sincp =>
					nstate <= sfetch;
					pc_inc <= '1';
					ptr_inc <= '1';
				when sdecp =>
					nstate <= sfetch;
					pc_inc  <= '1';
					ptr_dec <= '1';
				-- instrukce zmeny hodnoty bunky
				when sinc =>
					nstate <= sinc1;
					data_addr_sel <= '1';
					DATA_RDWR <= '1';
					DATA_EN <= '1';
				when sinc1 =>
					nstate <= sfetch;
					DATA_RDWR <= '0';
					DATA_EN <= '1';
					data_wdata_sel <= "11";
					pc_inc <= '1';
				when sdec =>
					nstate <= sdec1;
					data_addr_sel <= '1';
					DATA_RDWR <= '1';
					DATA_EN <= '1';
				when sdec1 =>
					nstate <= sfetch;
					DATA_RDWR <= '0';
					DATA_EN <= '1';
					data_wdata_sel <= "10";
					pc_inc <= '1';
				-- instrukce cyklu
				when swhile =>
					nstate <= swhile1;
					data_addr_sel <= '1';
					DATA_RDWR <= '1';
					DATA_EN <= '1';
				when swhile1 =>
					if (DATA_RDATA /= "00000000") then
						nstate <= sfetch;				
						pc_inc <= '1';
					else
						nstate <= swhile2;				
						pc_inc <= '1';
						cnt_inc <= '1';
					end if;
				when swhile2 =>
					if (cnt_reg = "00000000") then						
						nstate <= sfetch;
					else
						nstate <= swhile3;
						data_addr_sel <= '0';
						DATA_RDWR <= '1';
						DATA_EN <= '1';
					end if;
				when swhile3 =>		
					nstate <= swhile2;
					pc_inc <= '1';
					if (DATA_RDATA = X"5B") then
						cnt_inc <= '1';
					elsif (DATA_RDATA = X"5D") then
						cnt_dec <= '1';
					end if;
				-- -- -- -- -- -- -- -- --
				when snwhile =>
					nstate <= snwhile1;
					data_addr_sel <= '1';
					DATA_RDWR <= '1';
					DATA_EN <= '1';
				when snwhile1 =>
					if (DATA_RDATA = "00000000") then
						nstate <= sfetch;				
						pc_inc <= '1';
					else
						nstate <= snwhile2;				
						pc_dec <= '1';
						cnt_inc <= '1';
					end if;
				when snwhile2 =>
					if (cnt_reg = "00000000") then						
						nstate <= sfetch;
					else
						nstate <= snwhile3;
						data_addr_sel <= '0';
						DATA_RDWR <= '1';
						DATA_EN <= '1';
					end if;
				when snwhile3 =>
					nstate <= snwhile4;
					if (DATA_RDATA = X"5B") then
						cnt_dec <= '1';
					elsif (DATA_RDATA = X"5D") then
						cnt_inc <= '1';
					end if;
				when snwhile4 =>
					nstate <= snwhile2;
					if (cnt_reg = "00000000") then
						pc_inc <= '1';
					else
						pc_dec <= '1';
					end if;
				-- instrukce pro tisk/cteni
				when sprint =>
					if (OUT_BUSY = '1') then
						nstate <= sprint;
					else
						nstate <= sprint1;
						data_addr_sel <= '1';
						DATA_RDWR <= '1';
						DATA_EN <= '1';
					end if;
				when sprint1 =>
					nstate <= sfetch;
					data_addr_sel <= '1';
					OUT_DATA <= DATA_RDATA;
					OUT_WE <= '1';
					pc_inc <= '1';
				when sread =>
					IN_REQ <= '1';
					data_addr_sel <= '1';
					if (IN_VLD = '0') then
						nstate <= sread;
					else
						nstate <= sread1;
					end if;
				when sread1 =>
					nstate <= sfetch;
					data_addr_sel <= '1';
					IN_REQ <= '0';
					DATA_RDWR <= '0';
					data_wdata_sel <= "00";
					DATA_EN <= '1';
					pc_inc <= '1';
				-- instrukce ulozeni a nacteni hodnoty
				when sstore =>
					nstate <= sstore1;
					data_addr_sel <= '1';
					DATA_RDWR <= '1';
					DATA_EN <= '1';
				when sstore1 =>
					nstate <= sfetch;
					data_addr_sel <= '1';
					DATA_EN <= '0';
					tmp_ld <= '1';
					pc_inc <= '1';
				when sload =>
					nstate <= sfetch;
					DATA_RDWR <= '0';
					DATA_EN <= '1';
					data_wdata_sel <= "01";
					data_addr_sel <= '1';
					pc_inc <= '1';
				-- ostatni stavy
				when snop =>
					nstate <= sfetch;				
					pc_inc <= '1';
				when shalt =>
					nstate <= shalt;
				when others =>
					nstate <= shalt;
			end case;
		else
			nstate <= pstate;
		end if;
	end process;

end behavioral;
 