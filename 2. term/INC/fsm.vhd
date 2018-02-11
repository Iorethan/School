-- fsm.vhd: Finite State Machine
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;
-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity fsm is
port(
   CLK         : in  std_logic;
   RESET       : in  std_logic;

   -- Input signals
   KEY         : in  std_logic_vector(15 downto 0);
   CNT_OF      : in  std_logic;

   -- Output signals
   FSM_CNT_CE  : out std_logic;
   FSM_MX_MEM  : out std_logic;
   FSM_MX_LCD  : out std_logic;
   FSM_LCD_WR  : out std_logic;
   FSM_LCD_CLR : out std_logic
);
end entity fsm;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of fsm is
   type t_state is (T0, TA1, TA2, TA3, TA4, TA5, TA6, TA7, TA8, TA9, TB1, TB2,
   		    TB3, TB4, TB5, TB6, TB7, TB8, TB9, TB10, FAIL, PRINT_OK,
		    PRINT_FAIL, FINISH);
   signal present_state, next_state : t_state;

begin
-- -------------------------------------------------------
sync_logic : process(RESET, CLK)
begin
   if (RESET = '1') then
      present_state <= T0;
   elsif (CLK'event AND CLK = '1') then
      present_state <= next_state;
   end if;
end process sync_logic;

-- -------------------------------------------------------
next_state_logic : process(present_state, KEY, CNT_OF)
begin
   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when T0 =>
      next_state <= T0;
      if (KEY(8) = '1') then
         next_state <= TA1;
      elsif (KEY(7) = '1') then
         next_state <= TB1;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA1 =>
      next_state <= TA1;
      if (KEY(1) = '1') then
         next_state <= TA2;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA2 =>
      next_state <= TA2;
      if (KEY(2) = '1') then
         next_state <= TA3;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA3 =>
      next_state <= TA3;
      if (KEY(3) = '1') then
         next_state <= TA4;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA4 =>
      next_state <= TA4;
      if (KEY(9) = '1') then
         next_state <= TA5;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA5 =>
      next_state <= TA5;
      if (KEY(9) = '1') then
         next_state <= TA6;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA6 =>
      next_state <= TA6;
      if (KEY(3) = '1') then
         next_state <= TA7;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA7 =>
      next_state <= TA7;
      if (KEY(9) = '1') then
         next_state <= TA8;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA8 =>
      next_state <= TA8;
      if (KEY(3) = '1') then
         next_state <= TA9;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TA9 =>
      next_state <= TA9;
      if (KEY(15) = '1') then
         next_state <= PRINT_OK;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB1 =>
      next_state <= TB1;
      if (KEY(3) = '1') then
         next_state <= TB2;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB2 =>
      next_state <= TB2;
      if (KEY(1) = '1') then
         next_state <= TB3;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB3 =>
      next_state <= TB3;
      if (KEY(1) = '1') then
         next_state <= TB4;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB4 =>
      next_state <= TB4;
      if (KEY(5) = '1') then
         next_state <= TB5;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB5 =>
      next_state <= TB5;
      if (KEY(9) = '1') then
         next_state <= TB6;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB6 =>
      next_state <= TB6;
      if (KEY(4) = '1') then
         next_state <= TB7;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB7 =>
      next_state <= TB7;
      if (KEY(5) = '1') then
         next_state <= TB8;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB8 =>
      next_state <= TB8;
      if (KEY(4) = '1') then
         next_state <= TB9;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB9 =>
      next_state <= TB9;
      if (KEY(4) = '1') then
         next_state <= TB10;
      elsif (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when TB10 =>
      next_state <= TB10;
      if (KEY(15) = '1') then
         next_state <= PRINT_OK;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FAIL =>
      next_state <= FAIL;
      if (KEY(15) = '1') then
         next_state <= PRINT_FAIL;
      elsif (KEY(14 downto 0) /= "000000000000000") then
         next_state <= FAIL;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_OK =>
      next_state <= PRINT_OK;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_FAIL =>
      next_state <= PRINT_FAIL;
      if (CNT_OF = '1') then
         next_state <= FINISH;
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINISH =>
      next_state <= FINISH;
      if (KEY(15) = '1') then
         next_state <= T0; 
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      next_state <= T0;
   end case;
end process next_state_logic;

-- -------------------------------------------------------
output_logic : process(present_state, KEY)
begin
   FSM_CNT_CE     <= '0';
   FSM_MX_MEM     <= '0';
   FSM_MX_LCD     <= '0';
   FSM_LCD_WR     <= '0';
   FSM_LCD_CLR    <= '0';

   case (present_state) is
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_OK =>
      FSM_CNT_CE     <= '1';
      FSM_MX_MEM     <= '1';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when PRINT_FAIL =>
      FSM_CNT_CE     <= '1';
      FSM_MX_MEM     <= '0';
      FSM_MX_LCD     <= '1';
      FSM_LCD_WR     <= '1';
   -- - - - - - - - - - - - - - - - - - - - - - -
   when FINISH =>
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   -- - - - - - - - - - - - - - - - - - - - - - -
   when others =>
      if (KEY(14 downto 0) /= "000000000000000") then
         FSM_LCD_WR     <= '1';
      end if;
      if (KEY(15) = '1') then
         FSM_LCD_CLR    <= '1';
      end if;
   end case;
end process output_logic;

end architecture behavioral;

